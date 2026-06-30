#pragma once

// Boot-time App Store housekeeping (directories, temp cleanup, registry load).
class AppBootstrap {
 public:
  static void onBoot();
  static void cleanupTmpDir();
};
