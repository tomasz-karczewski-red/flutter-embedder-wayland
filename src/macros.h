// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <iostream>

#define FLWAY_DISALLOW_COPY(TypeName) TypeName(const TypeName &) = delete;

#define FLWAY_DISALLOW_ASSIGN(TypeName) void operator=(const TypeName &) = delete;

#define FLWAY_DISALLOW_COPY_AND_ASSIGN(TypeName)                                                                                                                                                                                               \
  FLWAY_DISALLOW_COPY(TypeName)                                                                                                                                                                                                                \
  FLWAY_DISALLOW_ASSIGN(TypeName)
