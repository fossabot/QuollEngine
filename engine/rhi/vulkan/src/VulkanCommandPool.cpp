#include "quoll/core/Base.h"
#include "quoll/core/Engine.h"

#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"
#include "VulkanError.h"

#include "VulkanLog.h"

namespace quoll::rhi {

VulkanCommandPool::VulkanCommandPool(VulkanDeviceObject &device,
                                     u32 queueFamilyIndex,
                                     const VulkanResourceRegistry &registry,
                                     const VulkanDescriptorPool &descriptorPool,
                                     DeviceStats &stats)
    : mDevice(device), mRegistry(registry), mDescriptorPool(descriptorPool),
      mStats(stats), mQueueFamilyIndex(queueFamilyIndex) {
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = mQueueFamilyIndex;

  checkForVulkanError(
      vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool),
      "Failed to create command pool");

  LOG_DEBUG_VK("Command pool created for queue family " << mQueueFamilyIndex,
               mCommandPool);
}

VulkanCommandPool::~VulkanCommandPool() {
  if (mCommandPool) {
    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    LOG_DEBUG_VK("Command pool destroyed", mCommandPool);
  }
}

std::vector<RenderCommandList>
VulkanCommandPool::createCommandLists(u32 count) {
  std::vector<VkCommandBuffer> commandBuffers(count);
  std::vector<RenderCommandList> renderCommandLists(count);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = mCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = count;

  checkForVulkanError(
      vkAllocateCommandBuffers(mDevice, &allocInfo, commandBuffers.data()),
      "Failed to allocate command buffers");

  for (usize i = 0; i < count; ++i) {
    renderCommandLists.at(i) =
        std::move(RenderCommandList(new VulkanCommandBuffer(
            commandBuffers.at(i), mRegistry, mDescriptorPool, mStats)));
  }

  LOG_DEBUG_VK("Command buffers allocated for queue family "
                   << mQueueFamilyIndex,
               mCommandPool);

  return std::move(renderCommandLists);
}

void VulkanCommandPool::freeCommandList(RenderCommandList &commandList) {
  auto *commandBuffer = dynamic_cast<rhi::VulkanCommandBuffer *>(
                            commandList.getNativeRenderCommandList().get())
                            ->getVulkanCommandBuffer();

  vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

} // namespace quoll::rhi
