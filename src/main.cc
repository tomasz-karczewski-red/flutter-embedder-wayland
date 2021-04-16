// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>

#include <string>
#include <vector>

#include "utils.h"
#include "debug.h"
#include "wayland_display.h"

static_assert(FLUTTER_ENGINE_VERSION == 1, "");

namespace flutter {

static void PrintUsage() {
  std::cerr << "Flutter Wayland Embedder" << std::endl << std::endl;
  std::cerr << "========================" << std::endl;
  std::cerr << "Usage: `" << GetExecutableName() << " <asset_bundle_path> <flutter_flags>`" << std::endl << std::endl;
  std::cerr << R"~(
This utility runs an instance of a Flutter application and renders using
Wayland core protocols.

The Flutter tools can be obtained at https://flutter.io/

asset_bundle_path: The Flutter application code needs to be snapshotted using
                   the Flutter tools and the assets packaged in the appropriate
                   location. This can be done for any Flutter application by
                   running `flutter build bundle` while in the directory of a
                   valid Flutter project. This should package all the code and
                   assets in the "build/flutter_assets" directory. Specify this
                   directory as the first argument to this utility.

    flutter_flags: Typically empty. These extra flags are passed directly to the
                   Flutter engine. To see all supported flags, run
                   `flutter_tester --help` using the test binary included in the
                   Flutter tools.

Supported environment variables:
     LANG=<string>
                   Value encoded as per setlocale(3) (e.g. "szl_PL.utf8") is
                   passed to the engine via UpdateLocales().

                   See also: https://man7.org/linux/man-pages/man3/setlocale.3.html
 
     FLUTTER_WAYLAND_PIXEL_RATIO=<double>
                   Overwrites the pixel aspect ratio reported
                   to the engine by FlutterEngineSendWindowMetricsEvent().

                   See also: https://api.flutter.dev/flutter/dart-ui/Window/devicePixelRatio.html

     FLUTTER_WAYLAND_MAIN_UI=<int>
                   Non-zero value enables grabbing all keys (even without having
                   a focus) using Xwayland keyboard grabbing protocol (assuming 
                   server implement this extension).

                   See also: https://github.com/wayland-project/wayland-protocols/tree/0a61d3516b10da4e65607a6dd97937ebedf6bcfa/unstable/xwayland-keyboard-grab

     FLUTTER_LAUNCHER_WAYLAND_DEBUG=<string>
                   where <string> can be any of syslog(3) prioritynames or its
                   unique abbreviation e.g. "err", "warning", "info" or "debug".

     FLUTTER_LAUNCHER_WAYLAND_CGROUP_MEMORY_PATH=<string>
                   if the app is run inside lxc container - path to where cgroup/memory memory.usage_in_bytes & cgroup.event_control are located
                   necessary for memory watcher to work

     FLUTTER_LAUNCHER_WAYLAND_MEMORY_WARNING_WATERMARK_BYTES=<string>
                   if FLUTTER_LAUNCHER_WAYLAND_CGROUP_MEMORY_PATH is defined, this specifies container memory usage levels at which the application will get
                   FlutterEngineNotifyLowMemoryWarning notifications; the format is comma-separated memory (in bytes) values, like:
                   "1000000,70000000,148478361,167038156,176318054"
                   At least one memory level needs to be defined for memory watcher to work. After each notification, there is 20sec cooldown period,
                   when additional FlutterEngineNotifyLowMemoryWarning notifications will not be called.
)~" << std::endl;
}

static bool Main(std::vector<std::string> args) {
  dbg_init();

  if (args.size() == 1) {
    dbgE("Invalid list of arguments\n");
    PrintUsage();
    return false;
  }

  const auto asset_bundle_path = args[1];

  if (!FlutterAssetBundleIsValid(asset_bundle_path)) {
    dbgE("Invalid Flutter Asset Bundle\n");
    PrintUsage();
    return false;
  }

  const size_t kWidth  = 1920;
  const size_t kHeight = 1080;

  const std::vector<std::string> flutter_args(args.begin() + 1, args.end());

  for (const auto &arg : flutter_args) {
    dbgI("Flutter arg: %s\n", arg.c_str());
  }

  WaylandDisplay display(kWidth, kHeight, asset_bundle_path, flutter_args);

  if (!display.IsValid()) {
    dbgI("Wayland display was not valid\n");
    return false;
  }

  return display.Run();
}

} // namespace flutter

int main(int argc, char *argv[]) {
  std::vector<std::string> args;
  for (int i = 0; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  return flutter::Main(std::move(args)) ? EXIT_SUCCESS : EXIT_FAILURE;
}
