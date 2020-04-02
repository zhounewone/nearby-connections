#ifndef CORE_INTERNAL_BANDWIDTH_UPGRADE_MANAGER_H_
#define CORE_INTERNAL_BANDWIDTH_UPGRADE_MANAGER_H_

#include <map>

#include "core/internal/bandwidth_upgrade_handler.h"
#include "core/internal/client_proxy.h"
#include "core/internal/endpoint_channel_manager.h"
#include "core/internal/endpoint_manager.h"
#include "core/internal/medium_manager.h"
#include "proto/connections/offline_wire_formats.pb.h"
#include "platform/port/string.h"
#include "platform/ptr.h"
#include "proto/connections_enums.pb.h"

namespace location {
namespace nearby {
namespace connections {

// Manages all known {@link BandwidthUpgradeHandler} implementations, delegating
// operations to the appropriate one as per the parameters passed in.
template <typename Platform>
class BandwidthUpgradeManager
    : public EndpointManager<Platform>::IncomingOfflineFrameProcessor {
 public:
  BandwidthUpgradeManager(
      Ptr<MediumManager<Platform> > medium_manager,
      Ptr<EndpointChannelManager<Platform> > endpoint_channel_manager,
      Ptr<EndpointManager<Platform> > endpoint_manager);
  ~BandwidthUpgradeManager() override;

  // This is the point on the initiator side where the
  // current_bandwidth_upgrade_handler_ is set.
  void initiateBandwidthUpgradeForEndpoint(
      Ptr<ClientProxy<Platform> > client_proxy, const string& endpoint_id,
      proto::connections::Medium medium);
  // This is the point on the non-initiator side where the
  // current_bandwidth_upgrade_handler_ is set.
  // @EndpointManagerReaderThread
  void processIncomingOfflineFrame(
      ConstPtr<OfflineFrame> offline_frame, const string& from_endpoint_id,
      Ptr<ClientProxy<Platform> > to_client_proxy,
      proto::connections::Medium current_medium) override;
  // @EndpointManagerReaderThread
  void processEndpointDisconnection(
      Ptr<ClientProxy<Platform> > client_proxy, const string& endpoint_id,
      Ptr<CountDownLatch> process_disconnection_barrier) override;

 private:
  bool setCurrentBandwidthUpgradeHandler(proto::connections::Medium medium);

  Ptr<EndpointManager<Platform> > endpoint_manager_;
  typedef std::map<proto::connections::Medium,
                   Ptr<BandwidthUpgradeHandler<Platform> > >
      BandwidthUpgradeHandlersMap;
  BandwidthUpgradeHandlersMap bandwidth_upgrade_handlers_;
  Ptr<BandwidthUpgradeHandler<Platform> > current_bandwidth_upgrade_handler_;
};

}  // namespace connections
}  // namespace nearby
}  // namespace location

#include "core/internal/bandwidth_upgrade_manager.cc"

#endif  // CORE_INTERNAL_BANDWIDTH_UPGRADE_MANAGER_H_