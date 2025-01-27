#pragma once

#include "quoll/rhi/RenderHandle.h"
#include "quoll/rhi/PipelineDescription.h"
#include "quoll/rhi/RenderPassDescription.h"
#include "quoll/rhi/RenderCommandList.h"
#include "quoll/rhi/BufferDescription.h"
#include "RenderGraphResource.h"

namespace quoll {

class RenderGraph;

enum class AttachmentType { Color, Depth, Resolve };

/**
 * @brief Render graph attachment data
 */
struct AttachmentData {
  /**
   * Attachment type
   */
  AttachmentType type = AttachmentType::Color;

  /**
   * Clear value
   */
  rhi::AttachmentClearValue clearValue;

  /**
   * Load operation
   */
  rhi::AttachmentLoadOp loadOp = rhi::AttachmentLoadOp::DontCare;

  /**
   * Store operation
   */
  rhi::AttachmentStoreOp storeOp = rhi::AttachmentStoreOp::DontCare;
};

/**
 * @brief Render target data
 */
struct RenderTargetData {
  /**
   * Texture
   */
  RenderGraphResource<rhi::TextureHandle> texture;

  /**
   * Source image layout
   */
  rhi::ImageLayout srcLayout{rhi::ImageLayout::Undefined};

  /**
   * Destination image layout
   */
  rhi::ImageLayout dstLayout{rhi::ImageLayout::Undefined};
};

/**
 * @brief Pass buffer data
 */
struct RenderGraphPassBufferData {
  /**
   * Buffer
   */
  rhi::BufferHandle buffer = rhi::BufferHandle::Null;

  /**
   * Buffer usage
   */
  rhi::BufferUsage usage = rhi::BufferUsage::None;
};

/**
 * @brief Render graph pass barrier
 */
struct RenderGraphPassBarrier {
  /**
   * Memory barriers
   */
  std::vector<rhi::MemoryBarrier> memoryBarriers;

  /**
   * Image barriers
   */
  std::vector<rhi::ImageBarrier> imageBarriers;

  /**
   * Buffer barriers
   */
  std::vector<rhi::BufferBarrier> bufferBarriers;
};

/**
 * @brief Render graph pass types
 */
enum class RenderGraphPassType { Graphics, Compute };

/**
 * @brief Render graph pass
 */
class RenderGraphPass {
  using ExecutorFn = std::function<void(rhi::RenderCommandList &, u32)>;
  friend RenderGraph;

public:
  /**
   * @brief Create render graph pass
   *
   * @param name Pass name
   * @param type Pass type
   */
  RenderGraphPass(StringView name, RenderGraphPassType type);

  /**
   * @brief Copy another render pass into this
   *
   * @param rhs Render pass to copy
   */
  RenderGraphPass(const RenderGraphPass &rhs) = default;

  /**
   * @brief Copy another render pass into this
   *
   * @param rhs Render pass to copy
   * @return This render pass
   */
  RenderGraphPass &operator=(const RenderGraphPass &rhs) = default;

  /**
   * @brief Move another render pass into this
   *
   * @param rhs Render pass to move
   */
  RenderGraphPass(RenderGraphPass &&rhs) = default;

  /**
   * @brief Move another render pass into this
   *
   * @param rhs Render pass to move
   * @return This render pass
   */
  RenderGraphPass &operator=(RenderGraphPass &&rhs) = default;

  /**
   * @brief Destroy render pass
   */
  ~RenderGraphPass() = default;

  /**
   * @brief Execute pass
   *
   * @param commandList Command list
   * @param frameIndex Frame index
   */
  void execute(rhi::RenderCommandList &commandList, u32 frameIndex);

  /**
   * @brief Set output texture
   *
   * @param handle Texture handle
   * @param type Attachment type
   * @param clearValue Clear value
   */
  void write(RenderGraphResource<rhi::TextureHandle> handle,
             AttachmentType type, const rhi::AttachmentClearValue &clearValue);

  /**
   * @brief Set input texture
   *
   * @param handle Texture handle
   */
  void read(RenderGraphResource<rhi::TextureHandle> handle);

  /**
   * @brief Set output buffer
   *
   * @param handle Buffer handle
   * @param usage Buffer usage
   */
  void write(rhi::BufferHandle handle, rhi::BufferUsage usage);

  /**
   * @brief Set input buffer
   *
   * @param handle Buffer handle
   * @param usage Buffer usage
   */
  void read(rhi::BufferHandle handle, rhi::BufferUsage usage);

  /**
   * @brief Set executor function
   *
   * @param executor Executor function
   */
  void setExecutor(const ExecutorFn &executor);

  /**
   * @brief Add pipeline to pass
   *
   * @param handle Pipeline handle
   */
  void addPipeline(rhi::PipelineHandle handle);

  /**
   * @brief Get pass name
   *
   * @return Pass name
   */
  inline const String &getName() const { return mName; }

  /**
   * @brief Get pass type
   *
   * @return Pass type
   */
  inline const RenderGraphPassType &getType() const { return mType; }

  /**
   * @brief Get texture inputs
   *
   * @return Texture inputs
   */
  inline const std::vector<RenderTargetData> &getTextureInputs() const {
    return mTextureInputs;
  }

  /**
   * @brief Get texture outputs
   *
   * @return Texture outputs
   */
  inline const std::vector<RenderTargetData> &getTextureOutputs() const {
    return mTextureOutputs;
  }

  /**
   * @brief Get buffer inputs
   *
   * @return Buffer inputs
   */
  inline const std::vector<RenderGraphPassBufferData> &getBufferInputs() const {
    return mBufferInputs;
  }

  /**
   * @brief Get render pass
   *
   * @return Render pass
   */
  inline rhi::RenderPassHandle getRenderPass() const { return mRenderPass; }

  /**
   * @brief Get framebuffer
   *
   * @return Framebuffer
   */
  inline rhi::FramebufferHandle getFramebuffer() const { return mFramebuffer; }

  /**
   * @brief Get pipelines
   *
   * @return Pipelines
   */
  inline const std::vector<rhi::PipelineHandle> &getPipelines() const {
    return mPipelines;
  }

  /**
   * @brief Get output buffers
   *
   * @return Output buffers
   */
  inline const std::vector<RenderGraphPassBufferData> &
  getBufferOutputs() const {
    return mBufferOutputs;
  }

  /**
   * @brief Get attachment data
   *
   * @return Attachment data
   */
  inline const std::vector<AttachmentData> &getAttachments() const {
    return mAttachments;
  }

  /**
   * @brief Get dimensions
   *
   * @return Dimensions
   */
  inline const glm::uvec3 &getDimensions() const { return mDimensions; }

  /**
   * @brief Get pass synchronization dependencies
   *
   * @return Pass syncronization dependencies
   */
  inline const RenderGraphPassBarrier &getSyncDependencies() const {
    return mDependencies;
  }

private:
  std::vector<AttachmentData> mAttachments;
  std::vector<RenderTargetData> mTextureOutputs;
  std::vector<RenderTargetData> mTextureInputs;

  std::vector<RenderGraphPassBufferData> mBufferInputs;
  std::vector<RenderGraphPassBufferData> mBufferOutputs;

  RenderGraphPassBarrier mDependencies;

  ExecutorFn mExecutor;

  std::vector<rhi::PipelineHandle> mPipelines;

  String mName;
  RenderGraphPassType mType;

  bool mCreated = false;

  // Graphics specific resources
  rhi::RenderPassHandle mRenderPass = rhi::RenderPassHandle::Null;
  rhi::FramebufferHandle mFramebuffer = rhi::FramebufferHandle::Null;
  glm::uvec3 mDimensions{};
};

} // namespace quoll
