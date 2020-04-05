#include "core/internal/base_endpoint_channel.h"

#include "platform/impl/default/default_platform.h"
#include "platform/pipe.h"
#include "proto/connections_enums.pb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace location {
namespace nearby {
namespace connections {
namespace {

class TestPlatform : public DefaultPlatform {
 public:
  static SystemClock* createSystemClock() { return nullptr; }

  static Ptr<AtomicBoolean> createAtomicBoolean(bool initial_value) {
    return Ptr<AtomicBoolean>();
  }

  template <typename T>
  static Ptr<AtomicReference<T>> createAtomicReference(const T& initial_value) {
    return Ptr<AtomicReference<T>>();
  }
};

class TestEndpointChannel : public BaseEndpointChannel<TestPlatform> {
 public:
  explicit TestEndpointChannel(Ptr<InputStream> input_stream)
      : BaseEndpointChannel("channel", input_stream, Ptr<OutputStream>()) {}

  MOCK_METHOD(proto::connections::Medium, getMedium, (), (override));
  MOCK_METHOD(void, closeImpl, (), (override));
};

using SamplePipe = Pipe<TestPlatform>;

TEST(BaseEndpointChannelTest, ReadAfterInputStreamClosed) {
  auto pipe = MakeRefCountedPtr(new SamplePipe());
  ScopedPtr<Ptr<InputStream>> input_stream(SamplePipe::createInputStream(pipe));
  ScopedPtr<Ptr<OutputStream>> output_stream(
      SamplePipe::createOutputStream(pipe));

  TestEndpointChannel test_channel(input_stream.get());

  // Close the output stream before trying to read from the input.
  output_stream->close();

  // Trying to read should fail gracefully with an IO error.
  ExceptionOr<ConstPtr<ByteArray>> result = test_channel.read();

  ASSERT_FALSE(result.ok());
  ASSERT_EQ(Exception::IO, result.exception());
}

}  // namespace
}  // namespace connections
}  // namespace nearby
}  // namespace location