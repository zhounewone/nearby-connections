#include "platform/base64_utils.h"

#include "strings/escaping.h"
#include "absl/strings/escaping.h"

namespace location {
namespace nearby {

std::string Base64Utils::encode(ConstPtr<ByteArray> bytes) {
  std::string base64_string;

  if (!bytes.isNull()) {
    absl::WebSafeBase64Escape(std::string(bytes->getData(), bytes->size()),
                              &base64_string);
  }

  return base64_string;
}

std::string Base64Utils::encode(const ByteArray& bytes) {
  std::string base64_string;
  absl::WebSafeBase64Escape(std::string(bytes.getData(), bytes.size()),
                            &base64_string);

  return base64_string;
}

std::string Base64Utils::encode(const std::string& input) {
  std::string base64_string;
  absl::WebSafeBase64Escape(input, &base64_string);

  return base64_string;
}

template<>
Ptr<ByteArray> Base64Utils::decode(const std::string& base64_string) {
  std::string decoded_string;
  if (!absl::WebSafeBase64Unescape(base64_string, &decoded_string)) {
    return Ptr<ByteArray>();
  }

  return MakePtr(new ByteArray(decoded_string.data(), decoded_string.size()));
}

template<>
ByteArray Base64Utils::decode(const std::string& base64_string) {
  std::string decoded_string;
  if (!absl::WebSafeBase64Unescape(base64_string, &decoded_string)) {
    return ByteArray();
  }

  return ByteArray(decoded_string.data(), decoded_string.size());
}

}  // namespace nearby
}  // namespace location