#pragma once

#include "VulkanHeaders.h"
#include <vk_mem_alloc.h>

#include "VulkanCommandPool.h"
#include "VulkanSwapchain.h"
#include "VulkanDeviceObject.h"
#include "VulkanQueue.h"
#include "VulkanFrameManager.h"

#include "quoll/rhi/RenderCommandList.h"

namespace quoll::rhi {

/**
 * @brief Vulkan render context
 */
class VulkanRenderContext {
public:
  /**
   * @brief Create render context
   *
   * Creates semaphores, fences, and command buffers
   * for rendering
   *
   * @param device Vulkan device
   * @param pool Command pool
   * @param graphicsQueue Graphics queue
   * @param presentQueue Present queue
   */
  VulkanRenderContext(VulkanDeviceObject &device, VulkanCommandPool &pool,
                      VulkanQueue &graphicsQueue, VulkanQueue &presentQueue);

  /**
   * @brief Present to screen
   *
   * @param frameManager Frame manager
   * @param swapchain Vulkan swapchain
   * @param imageIdx Swapchain image index
   * @return Present queue submit result
   */
  VkResult present(VulkanFrameManager &frameManager,
                   const VulkanSwapchain &swapchain, u32 imageIdx);

  /**
   * @brief Begin Rendering
   *
   * Waits for fences and semaphores; and begins command buffers
   *
   * @param frameManager Frame manager
   * @return Command buffer for current frame
   */
  RenderCommandList &beginRendering(VulkanFrameManager &frameManager);

  /**
   * @brief End rendering
   *
   * Ends command buffer and submits it to the graphics queue
   *
   * @param frameManager Frame manager
   */
  void endRendering(VulkanFrameManager &frameManager);

private:
  std::vector<RenderCommandList> mRenderCommandLists;

  VulkanQueue &mGraphicsQueue;
  VulkanQueue &mPresentQueue;
  VulkanDeviceObject &mDevice;
};

} // namespace quoll::rhi
