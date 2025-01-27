#pragma once

#include "NativeRenderCommandListInterface.h"

namespace quoll::rhi {

/**
 * @brief Render command list
 *
 * Public interface over native
 * command list
 */
class RenderCommandList {
public:
  /**
   * @brief Default constructor
   */
  RenderCommandList() = default;

  /**
   * @brief Default destructor
   */
  ~RenderCommandList() = default;

  /**
   * @brief Create render command list
   *
   * @param nativeRenderCommandList Native render command list
   */
  RenderCommandList(NativeRenderCommandListInterface *nativeRenderCommandList)
      : mNativeRenderCommandList(nativeRenderCommandList) {}

  /**
   * @brief Move constructor
   *
   * @param rhs Other command list
   */
  RenderCommandList(RenderCommandList &&rhs) noexcept {
    mNativeRenderCommandList = std::move(rhs.mNativeRenderCommandList);
  }

  /**
   * @brief Move equality operator
   *
   * @param rhs Other render command list
   * @return This render command list
   */
  RenderCommandList &operator=(RenderCommandList &&rhs) noexcept {
    mNativeRenderCommandList = std::move(rhs.mNativeRenderCommandList);
    return *this;
  }

  RenderCommandList(const RenderCommandList &) = delete;
  RenderCommandList &operator=(const RenderCommandList &) = delete;

  /**
   * @brief Get native render command list
   *
   * @return Native render command list
   */
  inline const std::unique_ptr<NativeRenderCommandListInterface> &
  getNativeRenderCommandList() const {
    return mNativeRenderCommandList;
  }

  /**
   * @brief Begin render pass
   *
   * @param renderPass Render pass
   * @param framebuffer Framebuffer
   * @param renderAreaOffset Render area offset
   * @param renderAreaSize Render area size
   */
  inline void beginRenderPass(rhi::RenderPassHandle renderPass,
                              FramebufferHandle framebuffer,
                              const glm::ivec2 &renderAreaOffset,
                              const glm::uvec2 &renderAreaSize) {
    mNativeRenderCommandList->beginRenderPass(renderPass, framebuffer,
                                              renderAreaOffset, renderAreaSize);
  }

  /**
   * @brief End render pass
   */
  inline void endRenderPass() { mNativeRenderCommandList->endRenderPass(); }

  /**
   * @brief Bind pipeline
   *
   * @param pipeline Pipeline
   */
  void bindPipeline(PipelineHandle pipeline) {
    mNativeRenderCommandList->bindPipeline(pipeline);
  }

  /**
   * @brief Bind descriptor
   *
   * @param pipeline Pipeline
   * @param firstSet First set
   * @param descriptor Descriptor
   * @param dynamicOffsets Dynamic offsets
   */
  inline void bindDescriptor(PipelineHandle pipeline, u32 firstSet,
                             const Descriptor &descriptor,
                             std::span<u32> dynamicOffsets = {}) {
    mNativeRenderCommandList->bindDescriptor(pipeline, firstSet, descriptor,
                                             dynamicOffsets);
  }

  /**
   * @brief Bind vertex buffers
   *
   * @param buffers Vertex buffers
   * @param offsets Vertex buffer binding offsets
   */
  inline void bindVertexBuffers(const std::span<const BufferHandle> buffers,
                                const std::span<const u64> offsets) {
    mNativeRenderCommandList->bindVertexBuffers(buffers, offsets);
  }

  /**
   * @brief Bind index buffer
   *
   * @param buffer Index buffer
   * @param indexType Index buffer data type
   */
  inline void bindIndexBuffer(BufferHandle buffer, IndexType indexType) {
    mNativeRenderCommandList->bindIndexBuffer(buffer, indexType);
  }

  /**
   * @brief Push constants
   *
   * @param pipeline Pipeline
   * @param shaderStage Shader stage
   * @param offset Offset
   * @param size Size
   * @param data Data
   */
  inline void pushConstants(PipelineHandle pipeline, ShaderStage shaderStage,
                            u32 offset, u32 size, void *data) {
    mNativeRenderCommandList->pushConstants(pipeline, shaderStage, offset, size,
                                            data);
  }

  /**
   * @brief Draw
   *
   * @param vertexCount Vertex count
   * @param firstVertex First vertex
   * @param instanceCount Instance count
   * @param firstInstance First instance
   */
  inline void draw(u32 vertexCount, u32 firstVertex, u32 instanceCount = 1,
                   u32 firstInstance = 0) {
    mNativeRenderCommandList->draw(vertexCount, firstVertex, instanceCount,
                                   firstInstance);
  }

  /**
   * @brief Draw indexed
   *
   * @param indexCount Index count
   * @param firstIndex Offset of first index
   * @param vertexOffset Vertex offset
   * @param instanceCount Instance count
   * @param firstInstance First instance
   */
  inline void drawIndexed(u32 indexCount, u32 firstIndex, i32 vertexOffset,
                          u32 instanceCount = 1, u32 firstInstance = 0) {
    mNativeRenderCommandList->drawIndexed(indexCount, firstIndex, vertexOffset,
                                          instanceCount, firstInstance);
  }

  /**
   * @brief Dispatch compute work
   *
   * @param groupCountX Number of groups to dispatch in X direction
   * @param groupCountY Number of groups to dispatch in Y direction
   * @param groupCountZ Number of groups to dispatch in Z direction
   */
  inline void dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ) {
    mNativeRenderCommandList->dispatch(groupCountX, groupCountY, groupCountZ);
  }

  /**
   * @brief Set viewport
   *
   * @param offset Viewport offset
   * @param size Viewport size
   * @param depthRange Viewport depth range
   */
  inline void setViewport(const glm::vec2 &offset, const glm::vec2 &size,
                          const glm::vec2 &depthRange) {
    mNativeRenderCommandList->setViewport(offset, size, depthRange);
  }

  /**
   * @brief Set scissor
   *
   * @param offset Scissor offset
   * @param size Scissor size
   */
  inline void setScissor(const glm::ivec2 &offset, const glm::uvec2 &size) {
    mNativeRenderCommandList->setScissor(offset, size);
  }

  /**
   * @brief Pipeline barrier
   *
   * @param memoryBarriers Memory barriers
   * @param imageBarriers Image barriers
   * @param bufferBarriers Buffer barriers
   */
  inline void pipelineBarrier(std::span<MemoryBarrier> memoryBarriers,
                              std::span<ImageBarrier> imageBarriers,
                              std::span<BufferBarrier> bufferBarriers) {
    mNativeRenderCommandList->pipelineBarrier(memoryBarriers, imageBarriers,
                                              bufferBarriers);
  }

  /**
   * @brief Copy texture to buffer
   *
   * @param srcTexture Source texture
   * @param dstBuffer Destination buffer
   * @param copyRegions Copy regions
   */
  inline void copyTextureToBuffer(TextureHandle srcTexture,
                                  BufferHandle dstBuffer,
                                  std::span<CopyRegion> copyRegions) {
    mNativeRenderCommandList->copyTextureToBuffer(srcTexture, dstBuffer,
                                                  copyRegions);
  }

  /**
   * @brief Copy buffer to texture
   *
   * @param srcBuffer Source buffer
   * @param dstTexture Destination texture
   * @param copyRegions Copy regions
   */
  inline void copyBufferToTexture(BufferHandle srcBuffer,
                                  TextureHandle dstTexture,
                                  std::span<CopyRegion> copyRegions) {
    mNativeRenderCommandList->copyBufferToTexture(srcBuffer, dstTexture,
                                                  copyRegions);
  }

  /**
   * @brief Blit texture
   *
   * @param source Source texture
   * @param destination Destination texture
   * @param regions Blit regions
   * @param filter Filter
   */
  inline void blitTexture(TextureHandle source, TextureHandle destination,
                          std::span<BlitRegion> regions, Filter filter) {
    mNativeRenderCommandList->blitTexture(source, destination, regions, filter);
  }

private:
  std::unique_ptr<NativeRenderCommandListInterface> mNativeRenderCommandList;
};

} // namespace quoll::rhi
