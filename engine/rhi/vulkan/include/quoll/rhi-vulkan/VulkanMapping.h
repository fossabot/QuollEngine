#pragma once

#include "quoll/rhi/RenderPassDescription.h"
#include "quoll/rhi/PipelineDescription.h"
#include "quoll/rhi/Descriptor.h"
#include "quoll/rhi/AccessFlags.h"
#include "quoll/rhi/ImageLayout.h"
#include "quoll/rhi/StageFlags.h"
#include "quoll/rhi/IndexType.h"
#include "quoll/rhi/Format.h"
#include "quoll/rhi/Filter.h"
#include "quoll/rhi/WrapMode.h"

#include "VulkanHeaders.h"

namespace quoll::rhi {

/**
 * @brief Vulkan mapper
 *
 * Maps engine specific types
 * with Vulkan types
 */
class VulkanMapping {
public:
  /**
   * @brief Get Vulkan primitive topology
   *
   * @param topology Primitive topology
   * @return Vulkan primitive topology
   */
  static VkPrimitiveTopology getPrimitiveTopology(PrimitiveTopology topology);

  /**
   * @brief Get Vulkan load operation
   *
   * @param loadOp Load operation
   * @return Vulkan load operation
   */
  static VkAttachmentLoadOp getAttachmentLoadOp(AttachmentLoadOp loadOp);

  /**
   * @brief Get Vulkan store operation
   *
   * @param storeOp Store operation
   * @return Vulkan store operation
   */
  static VkAttachmentStoreOp getAttachmentStoreOp(AttachmentStoreOp storeOp);

  /**
   * @brief Get Vulkan polygon mode
   *
   * @param polygonMode Polygon mode
   * @return Vulkan polygon mode
   */
  static VkPolygonMode getPolygonMode(PolygonMode polygonMode);

  /**
   * @brief Get Vulkan cull mode
   *
   * @param cullMode Cull mode
   * @return Vulkan cull mode
   */
  static VkCullModeFlags getCullMode(CullMode cullMode);

  /**
   * @brief Get Vulkan front face
   *
   * @param frontFace Front face
   * @return Vulkan front face
   */
  static VkFrontFace getFrontFace(FrontFace frontFace);

  /**
   * @brief Get Vulkan blend factor
   *
   * @param blendFactor Blend factor
   * @return Vulkan blend factor
   */
  static VkBlendFactor getBlendFactor(BlendFactor blendFactor);

  /**
   * @brief Get Vulkan blend operation
   *
   * @param blendOp Blend operation
   * @return Vulkan blend operation
   */
  static VkBlendOp getBlendOp(BlendOp blendOp);

  /**
   * @brief Get Vulkan compare operation
   *
   * @param compareOp Compare operation
   * @return Vulkan compare operation
   */
  static VkCompareOp getCompareOp(CompareOp compareOp);

  /**
   * @brief Get Vulkan stencil operation
   *
   * @param stencilOp Stencil operation
   * @return Vulkan stencil operation
   */
  static VkStencilOp getStencilOp(StencilOp stencilOp);

  /**
   * @brief Get Vulkan vertex input rate
   *
   * @param vertexInputRate Vertex input rate
   * @return Vulkan vertex input rate
   */
  static VkVertexInputRate getVertexInputRate(VertexInputRate vertexInputRate);

  /**
   * @brief Get Vulkan descriptor type
   *
   * @param descriptorType Descriptor type
   * @return Vulkan descriptor type
   */
  static VkDescriptorType getDescriptorType(DescriptorType descriptorType);

  /**
   * @brief Get Vulkan access flags
   *
   * @param access Access
   * @return Vulkan access flags
   */
  static VkAccessFlags2 getAccessFlags(Access access);

  /**
   * @brief Get Vulkan image layout
   *
   * @param imageLayout Image layout
   * @return Vulkan image layout
   */
  static VkImageLayout getImageLayout(ImageLayout imageLayout);

  /**
   * @brief Get Vulkan pipeline bind point
   *
   * @param pipelineBindPoint Pipeline bind point
   * @return Vulkan pipeline bind point
   */
  static VkPipelineBindPoint
  getPipelineBindPoint(PipelineBindPoint pipelineBindPoint);

  /**
   * @brief Get Vulkan shader stage flags
   *
   * @param shaderStage Shader stage
   * @return Vulkan shader stage flags
   */
  static VkShaderStageFlags getShaderStageFlags(ShaderStage shaderStage);

  /**
   * @brief Get Vulkan pipeline stage flags
   *
   * @param pipelineStage Pipeline stage
   * @return Vulkan pipeline stage flags
   */
  static VkPipelineStageFlags2
  getPipelineStageFlags(PipelineStage pipelineStage);

  /**
   * @brief Get Vulkan index type
   *
   * @param indexType Index type
   * @return Vulkan index type
   */
  static VkIndexType getIndexType(IndexType indexType);

  /**
   * @brief Get Vulkan format
   *
   * @param format Format
   * @return Vulkan format
   */
  static VkFormat getFormat(Format format);

  /**
   * @brief Get Vulkan filter
   *
   * @param filter Filter
   * @return Vulkan filter
   */
  static VkFilter getFilter(Filter filter);

  /**
   * @brief Get Vulkan sampler address mode
   *
   * @param wrapMode Wrap mode
   * @return Vulkan sampler address mode
   */
  static VkSamplerAddressMode getAddressMode(WrapMode wrapMode);

  // Vulkan to RHI mapping
public:
  /**
   * @brief Get RHI descriptor type
   *
   * @param descriptorType Vulkan descriptor type
   * @return RHI descriptor type
   */
  static DescriptorType getDescriptorType(VkDescriptorType descriptorType);

  /**
   * @brief Get RHI shader stage
   *
   * @param stageFlags Vulkan shader stage flags
   * @return RHI shader stage
   */
  static ShaderStage getShaderStage(VkShaderStageFlags stageFlags);
};

} // namespace quoll::rhi
