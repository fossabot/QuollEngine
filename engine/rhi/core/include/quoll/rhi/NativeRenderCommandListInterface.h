#pragma once

#include "quoll/rhi/Descriptor.h"
#include "quoll/rhi/RenderHandle.h"
#include "quoll/rhi/PipelineBarrier.h"
#include "quoll/rhi/StageFlags.h"
#include "quoll/rhi/IndexType.h"
#include "quoll/rhi/Filter.h"
#include "quoll/rhi/CopyRegion.h"
#include "quoll/rhi/BlitRegion.h"

namespace quoll::rhi {

/**
 * @brief Native render command list interface
 */
class NativeRenderCommandListInterface {
public:
  /**
   * @brief Create native render command list interface
   */
  NativeRenderCommandListInterface() = default;

  /**
   * @brief Destructor
   */
  virtual ~NativeRenderCommandListInterface() = default;

  NativeRenderCommandListInterface(const NativeRenderCommandListInterface &) =
      delete;
  NativeRenderCommandListInterface &
  operator=(const NativeRenderCommandListInterface &) = delete;
  NativeRenderCommandListInterface(NativeRenderCommandListInterface &&) =
      delete;
  NativeRenderCommandListInterface &
  operator=(NativeRenderCommandListInterface &&) = delete;

  /**
   * @brief Begin render pass
   *
   * @param renderPass Render pass
   * @param framebuffer Framebuffer
   * @param renderAreaOffset Render area offset
   * @param renderAreaSize Render area size
   */
  virtual void beginRenderPass(rhi::RenderPassHandle renderPass,
                               FramebufferHandle framebuffer,
                               const glm::ivec2 &renderAreaOffset,
                               const glm::uvec2 &renderAreaSize) = 0;

  /**
   * @brief End render pass
   */
  virtual void endRenderPass() = 0;

  /**
   * @brief Bind pipeline
   *
   * @param pipeline Pipeline
   */
  virtual void bindPipeline(PipelineHandle pipeline) = 0;

  /**
   * @brief Bind descriptor
   *
   * @param pipeline Pipeline
   * @param firstSet First set
   * @param descriptor Descriptor
   * @param dynamicOffsets Dynamic offsets
   */
  virtual void bindDescriptor(PipelineHandle pipeline, u32 firstSet,
                              const Descriptor &descriptor,
                              std::span<u32> dynamicOffsets) = 0;

  /**
   * @brief Bind vertex buffers
   *
   * @param buffers Vertex buffers
   * @param offsets Vertex buffer binding offsets
   */
  virtual void bindVertexBuffers(const std::span<const BufferHandle> buffers,
                                 const std::span<const u64> offsets) = 0;

  /**
   * @brief Bind index buffer
   *
   * @param buffer Index buffer
   * @param indexType Index buffer data type
   */
  virtual void bindIndexBuffer(BufferHandle buffer, IndexType indexType) = 0;

  /**
   * @brief Push constants
   *
   * @param pipeline Pipeline
   * @param shaderStage Shader stage
   * @param offset Offset
   * @param size Size
   * @param data Data
   */
  virtual void pushConstants(PipelineHandle pipeline, ShaderStage shaderStage,
                             u32 offset, u32 size, void *data) = 0;

  /**
   * @brief Draw
   *
   * @param vertexCount Vertex count
   * @param firstVertex First vertex
   * @param instanceCount Instance count
   * @param firstInstance First instance
   */
  virtual void draw(u32 vertexCount, u32 firstVertex, u32 instanceCount,
                    u32 firstInstance) = 0;

  /**
   * @brief Draw indexed
   *
   * @param indexCount Index count
   * @param firstIndex Offset of first index
   * @param vertexOffset Vertex offset
   * @param instanceCount Instance count
   * @param firstInstance First instance
   */
  virtual void drawIndexed(u32 indexCount, u32 firstIndex, i32 vertexOffset,
                           u32 instanceCount, u32 firstInstance) = 0;

  /**
   * @brief Dispatch compute work
   *
   * @param groupCountX Number of groups to dispatch in X direction
   * @param groupCountY Number of groups to dispatch in Y direction
   * @param groupCountZ Number of groups to dispatch in Z direction
   */
  virtual void dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ) = 0;

  /**
   * @brief Set viewport
   *
   * @param offset Viewport offset
   * @param size Viewport size
   * @param depthRange Viewport depth range
   */
  virtual void setViewport(const glm::vec2 &offset, const glm::vec2 &size,
                           const glm::vec2 &depthRange) = 0;

  /**
   * @brief Set scissor
   *
   * @param offset Scissor offset
   * @param size Scissor size
   */
  virtual void setScissor(const glm::ivec2 &offset, const glm::uvec2 &size) = 0;

  /**
   * @brief Pipeline barrier
   *
   * @param memoryBarriers Memory barriers
   * @param imageBarriers Image barriers
   * @param bufferBarriers Buffer barriers
   */
  virtual void pipelineBarrier(std::span<MemoryBarrier> memoryBarriers,
                               std::span<ImageBarrier> imageBarriers,
                               std::span<BufferBarrier> bufferBarriers) = 0;

  /**
   * @brief Copy texture to buffer
   *
   * @param srcTexture Source texture
   * @param dstBuffer Destination buffer
   * @param copyRegions Copy regions
   */
  virtual void copyTextureToBuffer(TextureHandle srcTexture,
                                   BufferHandle dstBuffer,
                                   std::span<CopyRegion> copyRegions) = 0;

  /**
   * @brief Copy buffer to texture
   *
   * @param srcBuffer Source buffer
   * @param dstTexture Destination texture
   * @param copyRegions Copy regions
   */
  virtual void copyBufferToTexture(BufferHandle srcBuffer,
                                   TextureHandle dstTexture,
                                   std::span<CopyRegion> copyRegions) = 0;

  /**
   * @brief Blit texture
   *
   * @param source Source texture
   * @param destination Destination texture
   * @param regions Blit regions
   * @param filter Filter
   */
  virtual void blitTexture(TextureHandle source, TextureHandle destination,
                           std::span<BlitRegion> regions, Filter filter) = 0;
};

} // namespace quoll::rhi
