// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_GLFW_EVENT_LOOP_H_
#define FLUTTER_SHELL_PLATFORM_GLFW_EVENT_LOOP_H_

#include <chrono>
#include <deque>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#include <flutter_embedder.h>

namespace flutter {

// platform event loop, to handle flutter platform events
// adapted from flutter engine glfw embedder (flutter/shell/platform/glfw/flutter_glfw.cc)
class PlatformEventLoop {
public:
  using TaskExpiredCallback = std::function<void(const FlutterTask *)>;
  // Creates an event loop running on the given thread, calling
  // |on_task_expired| to run tasks.
  // notify_fd event descriptor will be used to wake up the main thread when events arrive
  PlatformEventLoop(std::thread::id main_thread_id, const TaskExpiredCallback &on_task_expired, int notify_fd);

  virtual ~PlatformEventLoop();

  // Disallow copy.
  PlatformEventLoop(const PlatformEventLoop &) = delete;
  PlatformEventLoop &operator=(const PlatformEventLoop &) = delete;

  // Returns if the current thread is the thread used by this event loop.
  bool RunsTasksOnCurrentThread() const;

  // processes expired events, and returns next wake up point or 0 if there are no events
  uint64_t ProcessEvents();

  // Posts a Flutter engine task to the event loop for delayed execution.
  void PostTask(FlutterTask flutter_task, uint64_t flutter_target_time_nanos);

protected:

  // Wakes the main thread
  void Wake();

  struct Task {
    uint64_t order;
    uint64_t fire_time;
    FlutterTask task;

    struct Comparer {
      bool operator()(const Task &a, const Task &b) {
        if (a.fire_time == b.fire_time) {
          return a.order > b.order;
        }
        return a.fire_time > b.fire_time;
      }
    };
  };
  std::thread::id main_thread_id_;
  TaskExpiredCallback on_task_expired_;
  std::mutex task_queue_mutex_;
  std::priority_queue<Task, std::deque<Task>, Task::Comparer> task_queue_;
  int notify_fd_;
};

} // namespace flutter

#endif // FLUTTER_SHELL_PLATFORM_GLFW_EVENT_LOOP_H_
