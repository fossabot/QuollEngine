#pragma once

#include "liquid/rhi/NativeDescriptor.h"

#include "VulkanDeviceObject.h"
#include "VulkanResourceRegistry.h"

namespace liquid::rhi {

/**
 * @brief Vulkan descriptor set
 */
class VulkanDescriptorSet : public NativeDescriptor {
public:
  /**
   * @brief Create Vulkan descriptor set
   *
   * @param device Vulkan device
   * @param registry Vulkan resource registry
   * @param descriptorSet Vulkan descriptor set
   */
  VulkanDescriptorSet(VulkanDeviceObject &device,
                      const VulkanResourceRegistry &registry,
                      VkDescriptorSet descriptorSet);

  /**
   * @brief Write textures
   *
   * @param binding Binding number
   * @param textures Textures
   * @param type Descriptor type
   * @param start Starting index
   */
  void write(uint32_t binding, const std::vector<TextureHandle> &textures,
             DescriptorType type, uint32_t start) override;

  /**
   * @brief Write buffers
   *
   * @param binding Binding number
   * @param buffers Buffers
   * @param type Descriptor type
   * @param start Starting index
   */
  void write(uint32_t binding, const std::vector<BufferHandle> &buffers,
             DescriptorType type, uint32_t start) override;

private:
  /**
   * @brief Write descriptor set
   *
   * @param binding Binding number
   * @param start Starting index
   * @param descriptorCount Descriptor count
   * @param type Descriptor type
   * @param imageInfos Image infos
   * @param bufferInfos Buffer infos
   */
  void write(uint32_t binding, uint32_t start, size_t descriptorCount,
             DescriptorType type, const VkDescriptorImageInfo *imageInfos,
             VkDescriptorBufferInfo *bufferInfos);

private:
  VulkanDeviceObject &mDevice;
  const VulkanResourceRegistry &mRegistry;
  VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
};

} // namespace liquid::rhi