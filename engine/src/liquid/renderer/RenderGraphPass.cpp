#include "liquid/core/Base.h"
#include "RenderGraphPass.h"

namespace liquid {

RenderGraphPass::RenderGraphPass(StringView name, RenderGraphPassType type)
    : mName(name), mType(type) {}

void RenderGraphPass::write(rhi::TextureHandle handle, AttachmentType type,
                            const rhi::AttachmentClearValue &clearValue) {
  mTextureOutputs.push_back({handle});
  mAttachments.push_back({type, clearValue});
}

void RenderGraphPass::read(rhi::TextureHandle handle) {
  mTextureInputs.push_back({handle});
}

void RenderGraphPass::write(rhi::BufferHandle handle, rhi::BufferUsage usage) {
  LIQUID_ASSERT(
      !BitwiseEnumContains(usage, liquid::rhi::BufferUsage::Vertex) &&
          !BitwiseEnumContains(usage, liquid::rhi::BufferUsage::Index) &&
          !BitwiseEnumContains(usage, liquid::rhi::BufferUsage::Indirect),
      "Buffers can only be written from Uniform or Storage");
  mBufferOutputs.push_back({handle, usage});
}

void RenderGraphPass::read(rhi::BufferHandle handle, rhi::BufferUsage usage) {
  if (mType == RenderGraphPassType::Compute) {
    LIQUID_ASSERT(
        !BitwiseEnumContains(usage, liquid::rhi::BufferUsage::Vertex) &&
            !BitwiseEnumContains(usage, liquid::rhi::BufferUsage::Index),
        "Compute pass can only read "
        "buffers from uniform, storage, "
        "or indirect");
  }
  mBufferInputs.push_back({handle, usage});
}

void RenderGraphPass::setExecutor(const ExecutorFn &executor) {
  mExecutor = executor;
}

VirtualPipelineHandle RenderGraphPass::addPipeline(
    const rhi::GraphicsPipelineDescription &description) {
  LIQUID_ASSERT(mType == RenderGraphPassType::Graphics,
                "Cannot create graphics pipeline in a non-graphics pass");

  return mRegistry.set(description);
}

VirtualComputePipelineHandle RenderGraphPass::addPipeline(
    const rhi::ComputePipelineDescription &description) {
  LIQUID_ASSERT(mType == RenderGraphPassType::Compute,
                "Cannot create compute pipeline in a non-compute pass");
  return mRegistry.set(description);
}

void RenderGraphPass::execute(rhi::RenderCommandList &commandList,
                              uint32_t frameIndex) {
  mExecutor(commandList, mRegistry, frameIndex);
}

} // namespace liquid
