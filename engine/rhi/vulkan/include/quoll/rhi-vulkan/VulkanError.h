#pragma once

#include "quoll/core/Engine.h"

namespace quoll::rhi {

/**
 * @brief Create Vulkan error message
 *
 * @param resultCode Vulkan result code
 * @param errorMessage Custom error message
 * @return Error message
 */
String createVulkanErrorMessage(VkResult resultCode, const String &errorMessage,
                                const String &debugName);

/**
 * @brief Throws Vulkan error if result is not success
 *
 * @param resultCode Vulkan result code
 * @param errorMessage Error message to assert when result code is not success
 * @param debugName Debug name
 */
inline void checkForVulkanError(VkResult resultCode, const String &errorMessage,
                                const String &debugName = "") {
  QuollAssert(resultCode == VK_SUCCESS,
              createVulkanErrorMessage(resultCode, errorMessage, debugName));
  if (resultCode != VK_SUCCESS) {
    Engine::getLogger().fatal()
        << createVulkanErrorMessage(resultCode, errorMessage, debugName);
    std::terminate();
  }
}

} // namespace quoll::rhi
