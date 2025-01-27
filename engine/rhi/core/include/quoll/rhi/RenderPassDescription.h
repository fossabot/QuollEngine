#pragma once

#include "quoll/rhi/RenderHandle.h"
#include "quoll/rhi/ImageLayout.h"
#include "quoll/rhi/PipelineBindPoint.h"

namespace quoll::rhi {

enum class AttachmentLoadOp { Load, Clear, DontCare };

enum class AttachmentStoreOp { Store, DontCare };

/**
 * @brief Depth stencil clear
 */
struct DepthStencilClear {
  /**
   * Depth clear value
   */
  f32 clearDepth = 0.0f;

  /**
   * Stencil clear value
   */
  u32 clearStencil = 0;
};

using AttachmentClearValue = std::variant<glm::vec4, DepthStencilClear>;

/**
 * @brief Render pass description
 */
struct RenderPassAttachmentDescription {
  /**
   * Attachment load operation
   */
  AttachmentLoadOp loadOp;

  /**
   * Attachment store operation
   */
  AttachmentStoreOp storeOp;

  /**
   * Attachment stencil load operation
   */
  AttachmentLoadOp stencilLoadOp;

  /**
   * Attachment stencil store operation
   */
  AttachmentStoreOp stencilStoreOp;

  /**
   * Attachment texture
   */
  TextureHandle texture = TextureHandle::Null;

  /**
   * Attachment image initial layout
   */
  ImageLayout initialLayout{ImageLayout::Undefined};

  /**
   * Attachment image layout
   */
  ImageLayout layout{ImageLayout::Undefined};

  /**
   * Attachment clear value
   */
  AttachmentClearValue clearValue;
};

/**
 * @brief Render pass description
 */
struct RenderPassDescription {
  /**
   * Color attachments
   */
  std::vector<RenderPassAttachmentDescription> colorAttachments;

  /**
   * Depth attachment
   */
  std::optional<RenderPassAttachmentDescription> depthAttachment;

  /**
   * Resolve attachment
   */
  std::optional<RenderPassAttachmentDescription> resolveAttachment;

  /**
   * Pipeline bind point
   */
  PipelineBindPoint bindPoint;

  /**
   * Debug name
   */
  String debugName;
};

} // namespace quoll::rhi
