#include <gtest/gtest.h>

#include <chrono>
#include <stdexcept>

#include <engine/task/task_context.hpp>
#include <userver/engine/async.hpp>
#include <userver/engine/deadline.hpp>
#include <userver/engine/single_consumer_event.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/engine/task/cancel.hpp>
#include <userver/engine/task/task_with_result.hpp>

#include <userver/utest/utest.hpp>

USERVER_NAMESPACE_BEGIN

// Functors defined in dtors should unwind though
UTEST(Cancel, UnwindWorksInDtorSubtask) {
  class DetachingRaii final {
   public:
    DetachingRaii(engine::SingleConsumerEvent& detach_event,
                  engine::TaskWithResult<void>& detached_task)
        : detach_event_(detach_event), detached_task_(detached_task) {}

    ~DetachingRaii() {
      detached_task_ = engine::AsyncNoSpan([] {
        engine::InterruptibleSleepFor(utest::kMaxTestWaitTime);
        engine::current_task::CancellationPoint();
        ADD_FAILURE() << "Cancelled task ran past cancellation point";
      });
      detach_event_.Send();
    }

   private:
    engine::SingleConsumerEvent& detach_event_;
    engine::TaskWithResult<void>& detached_task_;
  };

  engine::TaskWithResult<void> detached_task;
  engine::SingleConsumerEvent task_detached_event;
  auto task = engine::AsyncNoSpan(
      [&] { DetachingRaii raii(task_detached_event, detached_task); });
  ASSERT_TRUE(task_detached_event.WaitForEvent());
  task.Wait();

  detached_task.WaitFor(std::chrono::milliseconds(10));
  ASSERT_FALSE(detached_task.IsFinished());
  detached_task.SyncCancel();
}

UTEST(Cancel, CancelDuringInterruptibleSleep) {
  engine::SingleConsumerEvent task_started;

  auto task = engine::CriticalAsyncNoSpan([&] {
    EXPECT_FALSE(engine::current_task::IsCancelRequested());
    task_started.Send();

    engine::InterruptibleSleepFor(utest::kMaxTestWaitTime);
    EXPECT_TRUE(engine::current_task::ShouldCancel());
  });

  EXPECT_TRUE(task_started.WaitForEventFor(utest::kMaxTestWaitTime));
  task.RequestCancel();
  UEXPECT_NO_THROW(task.Get());
}

UTEST(Cancel, CancelBeforeInterruptibleSleep) {
  engine::current_task::GetCurrentTaskContext().RequestCancel(
      engine::TaskCancellationReason::kUserRequest);

  // The task should wake up from this sleep immediately, because it is
  // already cancelled
  engine::InterruptibleSleepFor(utest::kMaxTestWaitTime);
  EXPECT_TRUE(engine::current_task::ShouldCancel());
}

USERVER_NAMESPACE_END
