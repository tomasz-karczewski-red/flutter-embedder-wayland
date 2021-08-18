// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "event_loop.h"
#include <atomic>
#include <utility>
#include <climits>

#include "event_loop.h"

#include <atomic>
#include <utility>
#include <unistd.h>

namespace flutter {

PlatformEventLoop::PlatformEventLoop(std::thread::id main_thread_id, const TaskExpiredCallback &on_task_expired, int notify_fd)
    : main_thread_id_(main_thread_id)
    , on_task_expired_(std::move(on_task_expired))
    , notify_fd_(notify_fd) {
}

PlatformEventLoop::~PlatformEventLoop() = default;

bool PlatformEventLoop::RunsTasksOnCurrentThread() const {
  return std::this_thread::get_id() == main_thread_id_;
}

uint64_t PlatformEventLoop::ProcessEvents() {
  std::vector<FlutterTask> expired_tasks;

  // Process expired tasks.
  {
    std::lock_guard<std::mutex> lock(task_queue_mutex_);
    const uint64_t start_processing_time = FlutterEngineGetCurrentTime();
    while (!task_queue_.empty()) {
      const auto &top = task_queue_.top();
      // If this task (and all tasks after this) has not yet expired, there is
      // nothing more to do. Quit iterating.
      if (top.fire_time > start_processing_time) {
        break;
      }

      // Make a record of the expired task. Do NOT service the task here
      // because we are still holding onto the task queue mutex. We don't want
      // other threads to block on posting tasks onto this thread till we are
      // done processing expired tasks.
      expired_tasks.push_back(task_queue_.top().task);

      // Remove the tasks from the delayed tasks queue.
      task_queue_.pop();
    }
  }

  // Fire expired tasks.
  {
    // Flushing tasks here without holing onto the task queue mutex.
    for (const auto &task : expired_tasks) {
      on_task_expired_(&task);
    }
  }

  // return timestamp of next event or 0 if none
  {
    std::lock_guard<std::mutex> lock(task_queue_mutex_);
    if (task_queue_.empty()) {
      return 0;
    } else {
      return task_queue_.top().fire_time;
    }
  }
}

void PlatformEventLoop::PostTask(FlutterTask flutter_task, uint64_t flutter_target_time_nanos) {
  static std::atomic<uint64_t> sGlobalTaskOrder(0);

  Task task;
  task.order     = ++sGlobalTaskOrder;
  task.fire_time = flutter_target_time_nanos;
  task.task      = flutter_task;

  {
    std::lock_guard<std::mutex> lock(task_queue_mutex_);
    task_queue_.push(task);

    // Make sure the queue mutex is unlocked before waking up the loop. In case
    // the wake causes this thread to be descheduled for the primary thread to
    // process tasks, the acquisition of the lock on that thread while holding
    // the lock here momentarily till the end of the scope is a pessimization.
  }
  Wake();
}

void PlatformEventLoop::Wake() {
  ssize_t ret  = 0;
  uint64_t val = 1;
  do {
    ret = write(notify_fd_, &val, sizeof(val));
  } while (ret < 0 && errno == EAGAIN);
}

} // namespace flutter
