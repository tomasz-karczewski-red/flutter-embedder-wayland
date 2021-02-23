// Copyright 2018 The Flutter Authors. All rights reserved.
// Copyright 2019 Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <fstream>
#include <flutter_embedder.h>
#include "macros.h"

namespace flutter {

constexpr auto NSEC_PER_SEC = 1'000'000'000;

std::string GetICUDataPath();

std::string GetExecutableName();

std::string GetExecutableDirectory();

bool FileExistsAtPath(const std::string &path);

bool FlutterAssetBundleIsValid(const std::string &bundle_path);

std::string FlutterGetAppAotElfName();

bool FlutterSendMessage(FlutterEngine engine, const char *channel, const uint8_t *message, const size_t message_size);

struct MyFlutterLocale : public FlutterLocale {
  MyFlutterLocale() {
    struct_size   = sizeof(FlutterLocale);
    language_code = script_code = country_code = variant_code = nullptr;
  }

  ~MyFlutterLocale() {
    free((void *)language_code), language_code = nullptr;
    free((void *)country_code), country_code   = nullptr;
    free((void *)script_code), script_code     = nullptr;
    free((void *)variant_code), variant_code   = nullptr;
  }
  FLWAY_DISALLOW_COPY_AND_ASSIGN(MyFlutterLocale)
};

void FlutterParseLocale(const std::string &locale, MyFlutterLocale *fl);

template <typename T> T getEnv(const char *variable, T default_value);

static inline void timespec_from_nsec(struct timespec *a, int64_t b) {
  a->tv_sec  = b / NSEC_PER_SEC;
  a->tv_nsec = b % NSEC_PER_SEC;
}

static inline void timespec_from_msec(struct timespec *a, int64_t b) {
  timespec_from_nsec(a, b * 1'000'000);
}

} // namespace flutter
