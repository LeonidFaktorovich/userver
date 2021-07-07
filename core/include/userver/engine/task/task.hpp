#pragma once

/// @file userver/engine/task/task.hpp
/// @brief @copybrief engine::Task

#include <chrono>
#include <string>

#include <boost/smart_ptr/intrusive_ptr.hpp>

#include <userver/engine/deadline.hpp>
#include <userver/engine/exception.hpp>
#include <userver/engine/task/cancel.hpp>
#include <userver/engine/task/task_processor_fwd.hpp>
#include <userver/utils/clang_format_workarounds.hpp>

namespace engine {
namespace ev {
class ThreadControl;
}  // namespace ev
namespace impl {
class TaskContext;
class TaskContextHolder;
}  // namespace impl

/// Asynchronous task
class USERVER_NODISCARD Task {
 public:
  /// Task importance
  enum class Importance {
    /// Normal task
    kNormal,
    /// Critical task, cannot be cancelled due to task processor overload
    kCritical,
  };

  /// Task state
  enum class State {
    kInvalid,    ///< Unusable
    kNew,        ///< just created, not registered with task processor
    kQueued,     ///< awaits execution
    kRunning,    ///< executing user code
    kSuspended,  ///< suspended, e.g. waiting for blocking call to complete
    kCancelled,  ///< exited user code because of external request
    kCompleted,  ///< exited user code with return or throw
  };

  /// @brief Default constructor
  ///
  /// Creates an invalid task.
  Task();

  /// @brief Destructor
  ///
  /// When the task is still valid and is not finished, cancels it and waits
  /// until it finishes
  virtual ~Task();

  Task(Task&&) noexcept;
  Task& operator=(Task&&) noexcept;

  /// @brief Checks whether this `Task` object owns
  /// an actual task (not `State::kInvalid`)
  ///
  /// An invalid task cannot be used. The task becomes invalid
  /// after each of the following calls:
  ///
  /// 1. `Task()`, the default constructor
  /// 2. `Detach()`
  /// 3. `Get()` (see `engine::TaskWithResult`)
  bool IsValid() const;

  /// Gets the task State
  State GetState() const;

  static const std::string& GetStateName(State state);

  /// Returns whether the task finished execution
  bool IsFinished() const;

  /// @brief Suspends execution until the task finishes or caller is cancelled.
  /// Can be called from coroutine context only. For non-coroutine context use
  /// BlockingWait().
  /// @throws WaitInterruptedException when `current_task::IsCancelRequested()`
  /// and no TaskCancellationBlockers are present.
  void Wait() const noexcept(false);

  /// @brief Suspends execution until the task finishes or after the specified
  /// timeout or until caller is cancelled
  /// @throws WaitInterruptedException when `current_task::IsCancelRequested()`
  /// and no TaskCancellationBlockers are present.
  template <typename Rep, typename Period>
  void WaitFor(const std::chrono::duration<Rep, Period>&) const noexcept(false);

  /// @brief Suspends execution until the task finishes or until the specified
  /// time point is reached or until caller is cancelled
  /// @throws WaitInterruptedException when `current_task::IsCancelRequested()`
  /// and no TaskCancellationBlockers are present.
  template <typename Clock, typename Duration>
  void WaitUntil(const std::chrono::time_point<Clock, Duration>&) const
      noexcept(false);

  /// @brief Suspends execution until the task finishes or until the specified
  /// deadline is reached or until caller is cancelled
  /// @throws WaitInterruptedException when `current_task::IsCancelRequested()`
  /// and no TaskCancellationBlockers are present.
  void WaitUntil(Deadline) const;

  /// @brief Detaches task, allowing it to continue execution out of scope
  /// @note After detach, Task becomes invalid
  void Detach() &&;

  /// Queues task cancellation request
  void RequestCancel();

  /// @brief Cancels the task and suspends execution until it is finished.
  /// Can be called from coroutine context only. For non-coroutine context use
  /// RequestCancel() + BlockingWait().
  void SyncCancel() noexcept;

  /// Gets task cancellation reason
  TaskCancellationReason CancellationReason() const;

  /// Waits for the task in non-coroutine context
  /// (e.g. non-TaskProcessor's std::thread).
  void BlockingWait() const;

 protected:
  /// @cond
  /// Constructor for internal use
  explicit Task(impl::TaskContextHolder&&);
  /// @endcond

  /// Marks task as invalid
  void Invalidate();

 private:
  void Terminate(TaskCancellationReason) noexcept;

  boost::intrusive_ptr<impl::TaskContext> context_;
};

namespace current_task {

/// Returns reference to the task processor executing the caller
TaskProcessor& GetTaskProcessor();

/// @cond
/// Returns ev thread handle, internal use only
ev::ThreadControl& GetEventThread();

/// Updates spurious wakeup statistics, internal use only
void AccountSpuriousWakeup();
/// @endcond

/// Returns task coroutine stack size
size_t GetStackSize();

}  // namespace current_task

template <typename Rep, typename Period>
void Task::WaitFor(const std::chrono::duration<Rep, Period>& duration) const
    noexcept(false) {
  WaitUntil(Deadline::FromDuration(duration));
}

template <typename Clock, typename Duration>
void Task::WaitUntil(const std::chrono::time_point<Clock, Duration>& until)
    const noexcept(false) {
  WaitUntil(Deadline::FromTimePoint(until));
}

}  // namespace engine