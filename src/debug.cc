// Copyright (c) 2021 Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//.
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//

#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <vector>
#include <mutex>

#ifndef SYSLOG_NAMES
#define SYSLOG_NAMES
#endif
#include <syslog.h>

#include "utils.h"
#include "debug.h"

namespace flutter {

static int debug_level;

int syslog2level(const std::string &priority) {
  const char *c_name = priority.c_str();
  for (size_t i = 0; prioritynames[i].c_name != NULL; i++) {
    if (strstr(prioritynames[i].c_name, c_name)) {
      return prioritynames[i].c_val;
    }
  }

  return LOG_INFO;
}

void dbg_init() {
  debug_level = syslog2level(getEnv("FLUTTER_LAUNCHER_WAYLAND_DEBUG", std::string("info")));
}

static const char *priority2prefix(const int priority) {
  static const struct {
    const int priority;
    const char *const prefix;
  } map[] = {
      {LOG_ERR, "ERROR: "},
      {LOG_WARNING, "WARNING: "},
      {LOG_INFO, "INFO: "},
      {LOG_DEBUG, "TRACE: "},
  };

  for (size_t i = 0; i < std::size(map); i++) {
    if (priority == map[i].priority) {
      return map[i].prefix;
    }
  }

  return "UNKNOWN: ";
}

static void vdbg(const int priority, const char *format, va_list args1) {
  va_list args2;
  va_copy(args2, args1);

  char buf[256];
  int rv = std::vsnprintf(buf, std::size(buf), format, args1);

  if (rv < 0) {
    va_end(args2);
    return;
  }

  if (static_cast<std::size_t>(rv) < std::size(buf)) {
    va_end(args2);

    printf("%s%s", priority2prefix(priority), buf);
    fflush(stdout);
  } else {
    const size_t len = rv + 1;
    std::vector<char> str(len);
    std::vsnprintf(str.data(), len, format, args2);
    va_end(args2);

    printf("%s%s", priority2prefix(priority), str.data());
    fflush(stdout);
  }
}

// clang-format off
#define dbg(type, priority)               \
void dbg##type(const char *format, ...) { \
  if (debug_level < (priority))           \
    return;                               \
                                          \
  va_list args;                           \
  va_start(args, format);                 \
  vdbg((priority), format, args);         \
  va_end(args);                           \
}

dbg(E, LOG_ERR)
dbg(W, LOG_WARNING)
dbg(I, LOG_INFO)
dbg(T, LOG_DEBUG)
// clang-format once

} // namespace flutter
