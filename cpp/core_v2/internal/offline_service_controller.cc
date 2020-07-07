#include "core_v2/internal/offline_service_controller.h"

#include <string>

namespace location {
namespace nearby {
namespace connections {

OfflineServiceController::~OfflineServiceController() {
  Stop();
}

void OfflineServiceController::Stop() {
  if (stop_.Set(true)) return;
  payload_manager_.DisconnectFromEndpointManager();
  pcp_manager_.DisconnectFromEndpointManager();
}

Status OfflineServiceController::StartAdvertising(
    ClientProxy* client, const std::string& service_id,
    const ConnectionOptions& options, const ConnectionRequestInfo& info) {
  return pcp_manager_.StartAdvertising(client, service_id, options, info);
}

void OfflineServiceController::StopAdvertising(ClientProxy* client) {
  pcp_manager_.StopAdvertising(client);
}

Status OfflineServiceController::StartDiscovery(
    ClientProxy* client, const std::string& service_id,
    const ConnectionOptions& options, const DiscoveryListener& listener) {
  return pcp_manager_.StartDiscovery(client, service_id, options, listener);
}

void OfflineServiceController::StopDiscovery(ClientProxy* client) {
  pcp_manager_.StopDiscovery(client);
}

Status OfflineServiceController::RequestConnection(
    ClientProxy* client, const std::string& endpoint_id,
    const ConnectionRequestInfo& info) {
  return pcp_manager_.RequestConnection(client, endpoint_id, info);
}

Status OfflineServiceController::AcceptConnection(
    ClientProxy* client, const std::string& endpoint_id,
    const PayloadListener& listener) {
  return pcp_manager_.AcceptConnection(client, endpoint_id, listener);
}

Status OfflineServiceController::RejectConnection(
    ClientProxy* client, const std::string& endpoint_id) {
  return pcp_manager_.RejectConnection(client, endpoint_id);
}

void OfflineServiceController::InitiateBandwidthUpgrade(
    ClientProxy* client, const std::string& endpoint_id) {
  // TODO(apolyudov): implement.
}

void OfflineServiceController::SendPayload(
    ClientProxy* client, const std::vector<std::string>& endpoint_ids,
    Payload payload) {
  payload_manager_.SendPayload(client, endpoint_ids, std::move(payload));
}

Status OfflineServiceController::CancelPayload(ClientProxy* client,
                                               std::int64_t payload_id) {
  return payload_manager_.CancelPayload(client, payload_id);
}

void OfflineServiceController::DisconnectFromEndpoint(
    ClientProxy* client, const std::string& endpoint_id) {
  endpoint_manager_.UnregisterEndpoint(client, endpoint_id);
}

}  // namespace connections
}  // namespace nearby
}  // namespace location