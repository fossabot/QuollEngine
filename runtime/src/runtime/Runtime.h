#pragma once

#include "LaunchConfig.h"

namespace liquid::runtime {

/**
 * @brief Runtime wrapper
 *
 * Starts all runtime processes
 */
class Runtime {
public:
  /**
   * @brief Create runtime
   *
   * @param config Launch config
   */
  Runtime(const LaunchConfig &config);

  /**
   * @brief Start runtime
   */
  void start();

private:
  LaunchConfig mConfig;
};

} // namespace liquid::runtime
