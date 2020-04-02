#ifndef PLATFORM_API_EXECUTOR_H_
#define PLATFORM_API_EXECUTOR_H_

namespace location {
namespace nearby {

// This abstract class is the superclass of all classes representing an
// Executor.
class Executor {
 public:
  virtual ~Executor() {}

  // https://docs.oracle.com/javase/8/docs/api/java/util/concurrent/ExecutorService.html#shutdown--
  virtual void shutdown() = 0;
};

}  // namespace nearby
}  // namespace location

#endif  // PLATFORM_API_EXECUTOR_H_