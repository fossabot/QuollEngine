#pragma once

#include "quoll/rhi-vulkan/VulkanDeviceObject.h"
#include "quoll/rhi-vulkan/VulkanResourceRegistry.h"
#include "quoll/rhi-vulkan/VulkanPipelineLayoutCache.h"

#include "quoll/rhi/PipelineDescription.h"

#include "VulkanHeaders.h"

namespace quoll::rhi {

/**
 * @brief Vulkan pipeline
 */
class VulkanPipeline {
public:
  /**
   * @brief Create graphics pipeline
   *
   * @param description Graphics pipeline description
   * @param device Vulkan device
   * @param registry Resource registry
   * @param pipelineLayoutCache Pipeline layout cache
   */
  VulkanPipeline(const GraphicsPipelineDescription &description,
                 VulkanDeviceObject &device,
                 const VulkanResourceRegistry &registry,
                 VulkanPipelineLayoutCache &pipelineLayoutCache);

  /**
   * @brief Create compute pipeline
   *
   * @param description Compute pipeline description
   * @param device Vulkan device
   * @param registry Resource registry
   * @param pipelineLayoutCache Pipeline layout cache
   */
  VulkanPipeline(const ComputePipelineDescription &description,
                 VulkanDeviceObject &device,
                 const VulkanResourceRegistry &registry,
                 VulkanPipelineLayoutCache &pipelineLayoutCache);

  /**
   * @brief Destructor
   *
   * Destroys Vulkan pipeline
   */
  ~VulkanPipeline();

  VulkanPipeline(const VulkanPipeline &) = delete;
  VulkanPipeline(VulkanPipeline &&) = delete;
  VulkanPipeline &operator=(const VulkanPipeline &) = delete;
  VulkanPipeline &operator=(VulkanPipeline &&) = delete;

  /**
   * @brief Get Vulkan pipeline
   *
   * @return Vulkan pipeline
   */
  inline VkPipeline getPipeline() const { return mPipeline; }

  /**
   * @brief Get Vulkan pipeline layout
   *
   * @return Vulkan pipeline layout
   */
  inline VkPipelineLayout getPipelineLayout() const { return mPipelineLayout; }

  /**
   * @brief Get descriptor layout at index
   *
   * @param index Descriptor layout index
   * @return Descriptor layout
   */
  inline const VkDescriptorSetLayout getDescriptorLayout(u32 index) const {
    return mDescriptorLayouts.at(index);
  }

  /**
   * @brief Get pipeline bind point
   *
   * @return Pipeline bind point
   */
  inline VkPipelineBindPoint getBindPoint() const { return mBindPoint; }

  /**
   * @brief Get debug name
   *
   * @return Debug name
   */
  const String &getDebugName() const { return mDebugName; }

private:
  /**
   * @brief Create pipeline layout
   *
   * @param shaders Shaders
   * @param registry Resource registry
   * @param pipelineLayoutCache Pipeline layout cache
   */
  void createLayout(const std::span<VulkanShader *> &shaders,
                    const VulkanResourceRegistry &registry,
                    VulkanPipelineLayoutCache &pipelineLayoutCache);

private:
  VulkanDeviceObject &mDevice;
  VkPipeline mPipeline = VK_NULL_HANDLE;
  VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
  VkPipelineBindPoint mBindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;

  std::unordered_map<u32, VkDescriptorSetLayout> mDescriptorLayouts;

  String mDebugName;
};

} // namespace quoll::rhi
