#pragma once

#include "VulkanDeviceObject.h"

namespace quoll::rhi {

/**
 * @brief Vulkan queue
 */
class VulkanQueue {
public:
  /**
   * @brief Create Vulkan queue
   *
   * @param device Vulkan device object
   * @param queueIndex Queue index
   */
  VulkanQueue(VulkanDeviceObject &device, u32 queueIndex);

  /**
   * @brief Get queue index
   *
   * @return Queue index
   */
  inline u32 getQueueIndex() const { return mQueueIndex; }

  /**
   * @brief Get Vulkan queue handle
   *
   * @return Vulkan queue handle
   */
  inline operator VkQueue() const { return mQueue; }

  /**
   * @brief Get Vulkan queue handle
   *
   * @return Vulkan queue handle
   */
  inline VkQueue getVulkanHandle() const { return mQueue; }

  /**
   * @brief Submit queue
   *
   * @param fence Submission fence
   * @param commandBufferInfos Command buffer infos
   * @param waitSemaphoreInfos Command buffer infos
   * @param signalSemaphoreInfos Command buffer infos
   */
  void submit(VkFence fence,
              std::span<VkCommandBufferSubmitInfo> commandBufferInfos,
              std::span<VkSemaphoreSubmitInfo> waitSemaphoreInfos,
              std::span<VkSemaphoreSubmitInfo> signalSemaphoreInfos);

  /**
   * @brief Wait for idle
   */
  void waitForIdle();

private:
  VkQueue mQueue = VK_NULL_HANDLE;
  u32 mQueueIndex = 0;
};

} // namespace quoll::rhi
