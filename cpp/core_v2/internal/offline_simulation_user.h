#ifndef CORE_V2_INTERNAL_OFFLINE_SIMULATION_USER_H_
#define CORE_V2_INTERNAL_OFFLINE_SIMULATION_USER_H_

#include <string>

#include "core_v2/internal/client_proxy.h"
#include "core_v2/internal/offline_service_controller.h"
#include "platform_v2/public/atomic_boolean.h"
#include "platform_v2/public/condition_variable.h"
#include "platform_v2/public/count_down_latch.h"
#include "platform_v2/public/future.h"
#include "gtest/gtest.h"
#include "absl/strings/string_view.h"

// Test-only class to help run end-to-end simulations for nearby connections
// protocol.
//
// This is a "standalone" version of PcpManager. It can run independently,
// provided MediumEnvironment has adequate support for all medium types in use.
namespace location {
namespace nearby {
namespace connections {

class OfflineSimulationUser {
 public:
  struct DiscoveredInfo {
    std::string endpoint_id;
    std::string endpoint_name;
    std::string service_id;

    bool Empty() const { return endpoint_id.empty(); }
    void Clear() { endpoint_id.clear(); }
  };

  explicit OfflineSimulationUser(absl::string_view device_name)
      : name_(device_name) {}
  virtual ~OfflineSimulationUser() = default;

  // Calls PcpManager::StartAdvertising().
  // If latch is provided, will call latch->CountDown() in the initiated_cb
  // callback.
  Status StartAdvertising(const std::string& service_id, CountDownLatch* latch);

  // Calls PcpManager::StopAdvertising().
  void StopAdvertising();

  // Calls PcpManager::StartDiscovery().
  // If latch is provided, will call latch->CountDown() in the endpoint_found_cb
  // callback.
  Status StartDiscovery(const std::string& service_id, CountDownLatch* latch);

  // Calls PcpManager::StopDiscovery().
  void StopDiscovery();

  // Calls PcpManager::RequestConnection().
  // If latch is provided, latch->CountDown() will be called in the initiated_cb
  // callback.
  Status RequestConnection(CountDownLatch* latch);

  // Calls PcpManager::AcceptConnection.
  // If latch is provided, latch->CountDown() will be called in the accepted_cb
  // callback.
  Status AcceptConnection(CountDownLatch* latch);

  // Calls PcpManager::RejectConnection.
  // If latch is provided, latch->CountDown() will be called in the rejected_cb
  // callback.
  Status RejectConnection(CountDownLatch* latch);

  // Unlike acceptance, rejection does not have to be mutual, in order to work.
  // This method will allow to synchronize on the remote rejection, without
  // performing a local rejection.
  // latch.CountDown() will be called in the rejected_cb callback.
  void ExpectRejectedConnection(CountDownLatch& latch) {
    reject_latch_ = &latch;
  }

  void ExpectPayload(CountDownLatch& latch) { payload_latch_ = &latch; }
  void ExpectDisconnect(CountDownLatch& latch) { disconnect_latch_ = &latch; }

  const DiscoveredInfo& GetDiscovered() const { return discovered_; }
  std::string GetName() const { return name_; }

  bool WaitForProgress(std::function<bool(const PayloadProgressInfo&)> pred,
                       absl::Duration timeout);

  Payload& GetPayload() { return payload_; }
  void SendPayload(Payload payload) {
    sender_payload_id_ = payload.GetId();
    ctrl_.SendPayload(&client_, {discovered_.endpoint_id}, std::move(payload));
  }

  Status CancelPayload() {
    if (sender_payload_id_) {
      return ctrl_.CancelPayload(&client_, sender_payload_id_);
    } else {
      return ctrl_.CancelPayload(&client_, payload_.GetId());
    }
  }

  void Disconnect();

  bool IsAdvertising() const { return client_.IsAdvertising(); }

  bool IsDiscovering() const { return client_.IsDiscovering(); }

  bool IsConnected() const {
    return client_.IsConnectedToEndpoint(discovered_.endpoint_id);
  }

  void Stop() {
    ctrl_.Stop();
  }

 protected:
  // ConnectionListener callbacks
  void OnConnectionInitiated(const std::string& endpoint_id,
                             const ConnectionResponseInfo& info,
                             bool is_outgoing);
  void OnConnectionAccepted(const std::string& endpoint_id);
  void OnConnectionRejected(const std::string& endpoint_id, Status status);
  void OnEndpointDisconnect(const std::string& endpoint_id);

  // DiscoveryListener callbacks
  void OnEndpointFound(const std::string& endpoint_id,
                       const std::string& endpoint_name,
                       const std::string& service_id);
  void OnEndpointLost(const std::string& endpoint_id);

  // PayloadListener callbacks
  void OnPayload(const std::string& endpoint_id, Payload payload);
  void OnPayloadProgress(const std::string& endpoint_id,
                         const PayloadProgressInfo& info);

  std::string service_id_;
  DiscoveredInfo discovered_;
  Mutex progress_mutex_;
  ConditionVariable progress_sync_{&progress_mutex_};
  PayloadProgressInfo progress_info_;
  Payload payload_;
  Payload::Id sender_payload_id_ = 0;
  CountDownLatch* initiated_latch_ = nullptr;
  CountDownLatch* accept_latch_ = nullptr;
  CountDownLatch* reject_latch_ = nullptr;
  CountDownLatch* found_latch_ = nullptr;
  CountDownLatch* lost_latch_ = nullptr;
  CountDownLatch* payload_latch_ = nullptr;
  CountDownLatch* disconnect_latch_ = nullptr;
  Future<bool>* future_ = nullptr;
  std::function<bool(const PayloadProgressInfo&)> predicate_;
  std::string name_;
  ConnectionOptions options_{.strategy = Strategy::kP2pCluster};
  ClientProxy client_;
  OfflineServiceController ctrl_;
};

}  // namespace connections
}  // namespace nearby
}  // namespace location

#endif  // CORE_V2_INTERNAL_OFFLINE_SIMULATION_USER_H_