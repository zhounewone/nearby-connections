#ifndef CORE_INTERNAL_ENDPOINT_CHANNEL_MANAGER_H_
#define CORE_INTERNAL_ENDPOINT_CHANNEL_MANAGER_H_

#include <map>

#include "core/internal/client_proxy.h"
#include "core/internal/endpoint_channel.h"
#include "core/internal/medium_manager.h"
#include "platform/api/ble.h"
#include "platform/api/bluetooth_classic.h"
#include "platform/api/lock.h"
#include "platform/port/string.h"
#include "platform/ptr.h"
#include "securegcm/d2d_connection_context_v1.h"

namespace location {
namespace nearby {
namespace connections {

// Manages the communication channels to all the remote endpoints with which we
// are interacting, including serving as a factory for creating said channels.
//
// The factory methods would be static, but for the fact that they need to use
// the MediumManager.
template <typename Platform>
class EndpointChannelManager {
 public:
  explicit EndpointChannelManager(Ptr<MediumManager<Platform> > medium_manager);
  ~EndpointChannelManager();

  Ptr<EndpointChannel> createOutgoingBluetoothEndpointChannel(
      const string& channel_name, Ptr<BluetoothSocket> bluetooth_socket);
  Ptr<EndpointChannel> createIncomingBluetoothEndpointChannel(
      const string& channel_name, Ptr<BluetoothSocket> bluetooth_socket);

  Ptr<EndpointChannel> createOutgoingBLEEndpointChannel(
      const string& channel_name, Ptr<BLESocket> ble_socket);
  Ptr<EndpointChannel> createIncomingBLEEndpointChannel(
      const string& channel_name, Ptr<BLESocket> ble_socket);

  // Registers the initial EndpointChannel to be associated with an endpoint;
  // if there already exists a previously-associated EndpointChannel, that will
  // be closed before continuing the registration.
  void registerChannelForEndpoint(Ptr<ClientProxy<Platform> > client_proxy,
                                  const string& endpoint_id,
                                  Ptr<EndpointChannel> endpoint_channel);

#ifdef BANDWIDTH_UPGRADE_MANAGER_IMPLEMENTED
  // Replaces the EndpointChannel to be associated with an endpoint from here on
  // in, transferring the encryption context from the previous EndpointChannel
  // to the newly-provided EndpointChannel.
  //
  // Returns the previous EndpointChannel, or null Ptr object if called out of
  // order.
  Ptr<EndpointChannel> replaceChannelForEndpoint(
      Ptr<ClientProxy<Platform> > client_proxy, const string& endpoint_id,
      Ptr<EndpointChannel> endpoint_channel);
#endif

  bool encryptChannelForEndpoint(
      const string& endpoint_id,
      Ptr<securegcm::D2DConnectionContextV1> encryption_context);

  // The returned Ptr will be owned (and destroyed) by the caller.
  Ptr<EndpointChannel> getChannelForEndpoint(const string& endpoint_id);

  // Returns true if 'endpoint_id' actually had a registered EndpointChannel.
  // IOW, a return of false signifies a no-op.
  bool unregisterChannelForEndpoint(const string& endpoint_id);

 private:
  // Tracks channel state for all endpoints. This includes what EndpointChannel
  // the endpoint is currently using and whether or not the EndpointChannel has
  // been encrypted yet.
  class ChannelState {
   public:
    ~ChannelState();

    // True if we have an 'encryption_context' for the endpoint.
    bool isEndpointEncrypted(const string& endpoint_id);

    // Stores a new EndpointChannel for the endpoint, returning the previous
    // one (if it existed).
    Ptr<EndpointChannel> updateChannelForEndpoint(
        const string& endpoint_id, Ptr<EndpointChannel> endpoint_channel);
    // Stores a new D2DConnectionContextV1 for the endpoint, returning the
    // previous one (if it existed).
    Ptr<securegcm::D2DConnectionContextV1> updateEncryptionContextForEndpoint(
        const string& endpoint_id,
        Ptr<securegcm::D2DConnectionContextV1> encryption_context);

    // Removes all knowledge of this endpoint, cleaning up as necessary.
    // Returns false if the endpoint was not found.
    bool removeEndpoint(const string& endpoint_id,
                        proto::connections::DisconnectionReason reason);

    // Gets the 'encryption_context' for the endpoint. Null if the endpoint was
    // not found, or if there is no 'encryption_context' yet.
    Ptr<securegcm::D2DConnectionContextV1> getEncryptionContextForEndpoint(
        const string& endpoint_id);
    // Gets the 'endpoint_channel' for the endpoint. Null if the endpoint was
    // not found.
    //
    // The returned Ptr will be owned (and destroyed) by the caller.
    Ptr<EndpointChannel> getChannelForEndpoint(const string& endpoint_id);

   private:
    struct EndpointMetaData {
      ~EndpointMetaData() {
        encryption_context.destroy();
        endpoint_channel.destroy();
      }

      Ptr<EndpointChannel> endpoint_channel;
      Ptr<securegcm::D2DConnectionContextV1> encryption_context;
    };

    // Endpoint ID -> EndpointMetadata. Contains everything we know about the
    // endpoint.
    typedef std::map<string, Ptr<EndpointMetaData> > EndpointIdToMetadataMap;
    EndpointIdToMetadataMap endpoint_id_to_metadata_;
  };

  void setActiveEndpointChannel(Ptr<ClientProxy<Platform> > client_proxy,
                                const string& endpoint_id,
                                Ptr<EndpointChannel> endpoint_channel);
  void encryptChannel(
      const string& endpoint_id, Ptr<EndpointChannel> endpoint_channel,
      Ptr<securegcm::D2DConnectionContextV1> encryption_context);

  ScopedPtr<Ptr<Lock> > lock_;

  Ptr<MediumManager<Platform> > medium_manager_;
  Ptr<ChannelState> channel_state_;
};

}  // namespace connections
}  // namespace nearby
}  // namespace location

#include "core/internal/endpoint_channel_manager.cc"

#endif  // CORE_INTERNAL_ENDPOINT_CHANNEL_MANAGER_H_