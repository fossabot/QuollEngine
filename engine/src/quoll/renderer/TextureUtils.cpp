#include "quoll/core/Base.h"
#include "TextureUtils.h"

namespace quoll {

void TextureUtils::copyDataToTexture(
    rhi::RenderDevice *device, void *source, rhi::TextureHandle destination,
    rhi::ImageLayout destinationLayout, u32 destinationLayers,
    const std::vector<TextureAssetLevel> &destinationLevels) {

  rhi::BufferDescription stagingBufferDesc{};
  stagingBufferDesc.size = getBufferSizeFromLevels(destinationLevels);
  stagingBufferDesc.data = source;
  stagingBufferDesc.usage = rhi::BufferUsage::TransferSource;

  auto stagingBuffer = device->createBuffer(stagingBufferDesc);

  auto commandList = device->requestImmediateCommandList();

  rhi::ImageBarrier barrier{};
  barrier.baseLevel = 0;
  barrier.levelCount = static_cast<u32>(destinationLevels.size());
  barrier.srcAccess = rhi::Access::None;
  barrier.dstAccess = rhi::Access::TransferWrite;
  barrier.srcLayout = rhi::ImageLayout::Undefined;
  barrier.dstLayout = rhi::ImageLayout::TransferDestinationOptimal;
  barrier.srcStage = rhi::PipelineStage::None;
  barrier.dstStage = rhi::PipelineStage::Transfer;
  barrier.texture = destination;

  std::array<rhi::ImageBarrier, 1> preBarriers{barrier};
  commandList.pipelineBarrier({}, preBarriers, {});

  std::vector<rhi::CopyRegion> copies(destinationLevels.size());
  for (usize i = 0; i < copies.size(); ++i) {
    auto &copy = copies.at(i);
    auto &dstLevel = destinationLevels.at(i);

    copy.bufferOffset = static_cast<u32>(dstLevel.offset);
    copy.imageBaseArrayLayer = 0;
    copy.imageLayerCount = destinationLayers;
    copy.imageExtent = {dstLevel.width, dstLevel.height, 1};
    copy.imageOffset = {0, 0, 0};
    copy.imageLevel = static_cast<u32>(i);
  }

  commandList.copyBufferToTexture(stagingBuffer.getHandle(), destination,
                                  copies);

  barrier.srcLayout = rhi::ImageLayout::TransferDestinationOptimal;
  barrier.dstLayout = destinationLayout;
  barrier.srcAccess = rhi::Access::TransferWrite;
  barrier.dstAccess = rhi::Access::None;
  barrier.srcStage = rhi::PipelineStage::Transfer;
  barrier.dstStage = rhi::PipelineStage::AllCommands;
  std::array<rhi::ImageBarrier, 1> postBarriers{barrier};
  commandList.pipelineBarrier({}, postBarriers, {});

  device->submitImmediate(commandList);

  device->destroyBuffer(stagingBuffer.getHandle());
}

void TextureUtils::copyTextureToData(
    rhi::RenderDevice *device, rhi::TextureHandle source,
    rhi::ImageLayout sourceLayout, u32 sourceLayers,
    const std::vector<TextureAssetLevel> &sourceLevels, void *destination) {
  rhi::BufferDescription stagingBufferDesc{};
  stagingBufferDesc.size = getBufferSizeFromLevels(sourceLevels);
  stagingBufferDesc.data = nullptr;
  stagingBufferDesc.usage = rhi::BufferUsage::TransferDestination;

  auto stagingBuffer = device->createBuffer(stagingBufferDesc);

  auto commandList = device->requestImmediateCommandList();

  rhi::ImageBarrier barrier{};
  barrier.baseLevel = 0;
  barrier.levelCount = static_cast<u32>(sourceLevels.size());
  barrier.srcAccess = rhi::Access::None;
  barrier.dstAccess = rhi::Access::TransferRead;
  barrier.srcLayout = rhi::ImageLayout::Undefined;
  barrier.dstLayout = rhi::ImageLayout::TransferSourceOptimal;
  barrier.texture = source;
  barrier.srcStage = rhi::PipelineStage::None;
  barrier.dstStage = rhi::PipelineStage::Transfer;

  std::array<rhi::ImageBarrier, 1> preBarriers{barrier};
  commandList.pipelineBarrier({}, preBarriers, {});

  std::vector<rhi::CopyRegion> copies(sourceLevels.size());
  for (usize i = 0; i < copies.size(); ++i) {
    auto &copy = copies.at(i);
    auto &srcLevel = sourceLevels.at(i);

    copy.bufferOffset = static_cast<u32>(srcLevel.offset);
    copy.imageBaseArrayLayer = 0;
    copy.imageLayerCount = sourceLayers;
    copy.imageExtent = {srcLevel.width, srcLevel.height, 1};
    copy.imageOffset = {0, 0, 0};
    copy.imageLevel = static_cast<u32>(i);
  }

  commandList.copyTextureToBuffer(source, stagingBuffer.getHandle(), copies);

  barrier.srcLayout = rhi::ImageLayout::TransferSourceOptimal;
  barrier.dstLayout = sourceLayout;
  barrier.srcAccess = rhi::Access::None;
  barrier.dstAccess = rhi::Access::None;
  barrier.srcStage = rhi::PipelineStage::Transfer;
  barrier.dstStage = rhi::PipelineStage::AllCommands;
  std::array<rhi::ImageBarrier, 1> postBarriers{barrier};
  commandList.pipelineBarrier({}, postBarriers, {});

  device->submitImmediate(commandList);

  void *data = stagingBuffer.map();
  memcpy(destination, data, stagingBufferDesc.size);
  stagingBuffer.unmap();

  device->destroyBuffer(stagingBuffer.getHandle());
}

void TextureUtils::generateMipMapsForTexture(rhi::RenderDevice *device,
                                             rhi::TextureHandle texture,
                                             rhi::ImageLayout layout,
                                             u32 layers, u32 levels, u32 width,
                                             u32 height) {
  auto commandList = device->requestImmediateCommandList();

  rhi::ImageBarrier barrier;
  barrier.texture = texture;
  barrier.baseLevel = 0;
  barrier.levelCount = levels;
  barrier.srcAccess = rhi::Access::None;
  barrier.dstAccess = rhi::Access::TransferWrite;
  barrier.srcLayout = rhi::ImageLayout::Undefined;
  barrier.dstLayout = rhi::ImageLayout::TransferDestinationOptimal;
  barrier.srcStage = rhi::PipelineStage::Transfer;
  barrier.dstStage = rhi::PipelineStage::Transfer;

  std::array<rhi::ImageBarrier, 1> preBarriers{barrier};
  commandList.pipelineBarrier({}, preBarriers, {});

  barrier.srcAccess = rhi::Access::TransferWrite;
  barrier.dstAccess = rhi::Access::TransferRead;
  barrier.srcLayout = rhi::ImageLayout::TransferDestinationOptimal;
  barrier.dstLayout = rhi::ImageLayout::TransferSourceOptimal;
  barrier.levelCount = 1;

  u32 mipWidth = width;
  u32 mipHeight = height;

  for (u32 i = 1; i < levels; ++i) {
    barrier.baseLevel = i - 1;
    barrier.srcStage = rhi::PipelineStage::Transfer;
    barrier.dstStage = rhi::PipelineStage::Transfer;
    std::array<rhi::ImageBarrier, 1> barriers{barrier};
    commandList.pipelineBarrier({}, barriers, {});

    rhi::BlitRegion region{};
    region.srcOffsets.at(0) = {0, 0, 0};
    region.srcOffsets.at(1) = {mipWidth, mipHeight, 1};
    region.srcLayerCount = layers;
    region.srcMipLevel = i - 1;
    region.dstOffsets.at(0) = {0, 0, 0};
    region.dstOffsets.at(1) = {mipWidth > 1 ? mipWidth / 2 : 1,
                               mipHeight > 1 ? mipHeight / 2 : 1, 1};
    region.dstLayerCount = layers;
    region.dstMipLevel = i;

    std::array<rhi::BlitRegion, 1> regions{region};
    commandList.blitTexture(texture, texture, regions, rhi::Filter::Nearest);

    if (mipWidth > 1) {
      mipWidth /= 2;
    }

    if (mipHeight > 1)
      mipHeight /= 2;
  }

  barrier.baseLevel = 0;
  barrier.levelCount = levels;
  barrier.srcLayout = rhi::ImageLayout::Undefined;
  barrier.dstLayout = layout;
  barrier.srcAccess = rhi::Access::None;
  barrier.dstAccess = rhi::Access::None;
  barrier.srcStage = rhi::PipelineStage::Transfer;
  barrier.dstStage = rhi::PipelineStage::AllCommands;

  std::array<rhi::ImageBarrier, 1> postBarriers{barrier};
  commandList.pipelineBarrier({}, postBarriers, {});

  device->submitImmediate(commandList);
}

usize TextureUtils::getBufferSizeFromLevels(
    const std::vector<TextureAssetLevel> &levels) {
  usize size = 0;
  for (auto &level : levels) {
    size += level.size;
  }

  return size;
}

} // namespace quoll
