#include "quoll/core/Base.h"
#include "MousePickingGraph.h"

#include "quoll/renderer/MeshVertexLayout.h"
#include "quoll/renderer/MeshRenderUtils.h"

namespace quoll::editor {

MousePickingGraph::MousePickingGraph(
    const std::array<SceneRendererFrameData, 2> &frameData,
    AssetRegistry &assetRegistry, RenderStorage &renderStorage)
    : mRenderStorage(renderStorage), mFrameData(frameData),
      mAssetRegistry(assetRegistry), mRenderGraph("MousePicking"),
      mBindlessParams{
          BindlessDrawParameters(renderStorage.getDevice()
                                     ->getDeviceInformation()
                                     .getLimits()
                                     .minUniformBufferOffsetAlignment),
          BindlessDrawParameters(renderStorage.getDevice()
                                     ->getDeviceInformation()
                                     .getLimits()
                                     .minUniformBufferOffsetAlignment)} {
  auto *device = mRenderStorage.getDevice();

  mRenderStorage.createShader("mouse-picking.sprite.vertex",
                              {"assets/shaders/mouse-picking-sprite.vert.spv"});
  mRenderStorage.createShader("mouse-picking.mesh.vertex",
                              {"assets/shaders/mouse-picking-mesh.vert.spv"});
  mRenderStorage.createShader(
      "mouse-picking.skinned-mesh.vertex",
      {"assets/shaders/mouse-picking-skinned-mesh.vert.spv"});
  mRenderStorage.createShader("mouse-picking.text.vertex",
                              {"assets/shaders/mouse-picking-text.vert.spv"});
  mRenderStorage.createShader(
      "mouse-picking.selector.fragment",
      {"assets/shaders/mouse-picking-selector.frag.spv"});

  Entity nullEntity{0};
  mSelectedEntityBuffer = renderStorage.createBuffer(
      {rhi::BufferUsage::Storage, sizeof(Entity), &nullEntity,
       rhi::BufferAllocationUsage::HostRead});

  rhi::BufferDescription defaultDesc{};
  defaultDesc.usage = rhi::BufferUsage::Storage;
  defaultDesc.size = sizeof(Entity) * mFrameData.at(0).getReservedSpace();
  defaultDesc.mapped = true;

  {
    auto desc = defaultDesc;
    desc.debugName = "sprite entities";
    mSpriteEntitiesBuffer = renderStorage.createBuffer(desc);
  }

  {
    auto desc = defaultDesc;
    desc.debugName = "mesh entities";
    mMeshEntitiesBuffer = renderStorage.createBuffer(desc);
  }

  {
    auto desc = defaultDesc;
    desc.debugName = "skinned mesh entities";
    mSkinnedMeshEntitiesBuffer = renderStorage.createBuffer(desc);
  }

  {
    auto desc = defaultDesc;
    desc.debugName = "text entities";
    mTextEntitiesBuffer = renderStorage.createBuffer(desc);
  }
}

void MousePickingGraph::execute(rhi::RenderCommandList &commandList,
                                const glm::vec2 &mousePos,
                                uint32_t frameIndex) {
  mFrameIndex = frameIndex;
  const auto &frameData = mFrameData.at(frameIndex);

  auto &textBounds = mMousePickingFrameData.at(frameIndex).textBounds;
  textBounds.clear();
  textBounds.reserve(frameData.getTexts().size());

  for (const auto &text : frameData.getTexts()) {
    glm::vec4 bounds(
        std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
        std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

    for (auto i = text.glyphStart; i < text.glyphStart + text.length; ++i) {
      const auto &glyph = frameData.getTextGlyphs().at(static_cast<size_t>(i));

      bounds.x = std::min(glyph.planeBounds.x, bounds.x);
      bounds.y = std::min(glyph.planeBounds.y, bounds.y);
      bounds.z = std::max(glyph.planeBounds.z, bounds.z);
      bounds.w = std::max(glyph.planeBounds.w, bounds.w);
    }

    textBounds.push_back(bounds);
  }

  mSpriteEntitiesBuffer.update(frameData.getSpriteEntities().data(),
                               frameData.getSpriteEntities().size() *
                                   sizeof(Entity));
  mTextEntitiesBuffer.update(frameData.getTextEntities().data(),
                             frameData.getTextEntities().size() *
                                 sizeof(Entity));

  {
    size_t offset = 0;
    auto *bufferData = static_cast<Entity *>(mMeshEntitiesBuffer.map());
    for (auto &[_, meshData] : frameData.getMeshGroups()) {
      memcpy(bufferData + offset, meshData.entities.data(),
             sizeof(Entity) * meshData.entities.size());
      offset += meshData.entities.size();
    }
    mMeshEntitiesBuffer.unmap();
  }

  {
    size_t offset = 0;
    auto *bufferData = static_cast<Entity *>(mSkinnedMeshEntitiesBuffer.map());
    for (auto &[_, meshData] : frameData.getSkinnedMeshGroups()) {
      memcpy(bufferData + offset, meshData.entities.data(),
             sizeof(Entity) * meshData.entities.size());
      offset += meshData.entities.size();
    }
    mSkinnedMeshEntitiesBuffer.unmap();
  }

  mMousePos = mousePos;

  if (mResized) {
    mRenderGraph.destroy(mRenderStorage);
    mRenderGraph = RenderGraph("Mouse picking");

    createRenderGraph();

    mRenderGraph.build(mRenderStorage);

    mResized = false;
  }

  mRenderGraph.execute(commandList, frameIndex);
}

Entity MousePickingGraph::getSelectedEntity() {
  auto selectedEntity = Entity::Null;

  auto *data = mSelectedEntityBuffer.map();
  memcpy(&selectedEntity, data, sizeof(Entity));
  mSelectedEntityBuffer.unmap();

  Entity nullEntity = Entity::Null;
  mSelectedEntityBuffer.update(&nullEntity, sizeof(Entity));

  mFrameIndex = std::numeric_limits<uint32_t>::max();

  return selectedEntity;
}

void MousePickingGraph::setFramebufferSize(glm::uvec2 size) {
  mFramebufferSize = size;
  mResized = true;
}

void MousePickingGraph::createRenderGraph() {
  for (auto &params : mBindlessParams) {
    params.destroy(mRenderStorage.getDevice());
  }

  rhi::TextureDescription depthBufferDesc{};
  depthBufferDesc.usage = rhi::TextureUsage::Depth | rhi::TextureUsage::Sampled;
  depthBufferDesc.width = mFramebufferSize.x;
  depthBufferDesc.height = mFramebufferSize.y;
  depthBufferDesc.layerCount = 1;
  depthBufferDesc.samples = 1;
  depthBufferDesc.format = rhi::Format::Depth32Float;
  depthBufferDesc.debugName = "Mouse picking depth stencil";

  auto depthBuffer = mRenderGraph.create(depthBufferDesc);

  auto &pass = mRenderGraph.addGraphicsPass("MousePicking");
  pass.write(depthBuffer, AttachmentType::Depth,
             rhi::DepthStencilClear({1.0f, 0}));

  // Sprites
  auto spritePipeline =
      mRenderStorage.addPipeline(rhi::GraphicsPipelineDescription{
          mRenderStorage.getShader("mouse-picking.sprite.vertex"),
          mRenderStorage.getShader("mouse-picking.selector.fragment"),
          {},
          rhi::PipelineInputAssembly{rhi::PrimitiveTopology::TriangleStrip},
          rhi::PipelineRasterizer{rhi::PolygonMode::Fill, rhi::CullMode::None,
                                  rhi::FrontFace::CounterClockwise},
          rhi::PipelineColorBlend{{}},
          {},
          {},
          "mouse picking sprite"});

  // Normal meshes
  auto meshPipeline =
      mRenderStorage.addPipeline(rhi::GraphicsPipelineDescription{
          mRenderStorage.getShader("mouse-picking.mesh.vertex"),
          mRenderStorage.getShader("mouse-picking.selector.fragment"),
          createMeshVertexLayout(),
          rhi::PipelineInputAssembly{rhi::PrimitiveTopology::TriangleList},
          rhi::PipelineRasterizer{rhi::PolygonMode::Fill, rhi::CullMode::Back,
                                  rhi::FrontFace::CounterClockwise},
          rhi::PipelineColorBlend{{}},
          {},
          {},
          "mouse picking mesh"});

  // Skinned meshes
  auto skinnedMeshPipeline =
      mRenderStorage.addPipeline(rhi::GraphicsPipelineDescription{
          mRenderStorage.getShader("mouse-picking.skinned-mesh.vertex"),
          mRenderStorage.getShader("mouse-picking.selector.fragment"),
          createSkinnedMeshPositionLayout(),
          rhi::PipelineInputAssembly{rhi::PrimitiveTopology::TriangleList},
          rhi::PipelineRasterizer{rhi::PolygonMode::Fill, rhi::CullMode::Back,
                                  rhi::FrontFace::CounterClockwise},
          rhi::PipelineColorBlend{{}},
          {},
          {},
          "mouse picking skinned mesh"});

  // Texts
  auto textPipeline =
      mRenderStorage.addPipeline(rhi::GraphicsPipelineDescription{
          mRenderStorage.getShader("mouse-picking.text.vertex"),
          mRenderStorage.getShader("mouse-picking.selector.fragment"),
          {},
          rhi::PipelineInputAssembly{rhi::PrimitiveTopology::TriangleStrip},
          rhi::PipelineRasterizer{rhi::PolygonMode::Fill, rhi::CullMode::None,
                                  rhi::FrontFace::CounterClockwise},
          rhi::PipelineColorBlend{{}},
          {},
          {},
          "mouse picking text"});

  pass.addPipeline(spritePipeline);
  pass.addPipeline(meshPipeline);
  pass.addPipeline(skinnedMeshPipeline);
  pass.addPipeline(textPipeline);

  struct MousePickingDrawParams {
    rhi::DeviceAddress selectedEntity;
    rhi::DeviceAddress camera;

    rhi::DeviceAddress spriteTransforms;
    rhi::DeviceAddress spriteEntities;

    rhi::DeviceAddress meshTransforms;
    rhi::DeviceAddress meshEntities;

    rhi::DeviceAddress skinnedMeshTransforms;
    rhi::DeviceAddress skinnedMeshEntities;
    rhi::DeviceAddress skeletons;

    rhi::DeviceAddress textTransforms;
    rhi::DeviceAddress textEntities;
    rhi::DeviceAddress glyphs;
  };

  size_t offset = 0;
  for (size_t i = 0; i < mBindlessParams.size(); ++i) {
    auto &frameData = mFrameData.at(i);
    offset = mBindlessParams.at(i).addRange(MousePickingDrawParams{
        mSelectedEntityBuffer.getAddress(), frameData.getCameraBuffer(),

        frameData.getSpriteTransformsBuffer(),
        mSpriteEntitiesBuffer.getAddress(),

        frameData.getMeshTransformsBuffer(), mMeshEntitiesBuffer.getAddress(),

        frameData.getSkinnedMeshTransformsBuffer(),
        mSkinnedMeshEntitiesBuffer.getAddress(), frameData.getSkeletonsBuffer(),

        frameData.getTextTransformsBuffer(), mTextEntitiesBuffer.getAddress(),
        frameData.getGlyphsBuffer()});
  }

  pass.setExecutor([this, spritePipeline, meshPipeline, skinnedMeshPipeline,
                    textPipeline, offset](rhi::RenderCommandList &commandList,
                                          uint32_t frameIndex) {
    auto &frameData = mFrameData.at(frameIndex);

    commandList.setScissor(glm::ivec2(mMousePos), glm::uvec2(1, 1));

    std::array<uint32_t, 1> offsets{static_cast<uint32_t>(offset)};

    // Sprite
    {
      commandList.bindPipeline(spritePipeline);
      commandList.bindDescriptor(spritePipeline, 0,
                                 mBindlessParams.at(frameIndex).getDescriptor(),
                                 offsets);

      commandList.draw(
          4, 0, static_cast<uint32_t>(frameData.getSpriteEntities().size()), 0);
    }

    // Mesh
    {
      commandList.bindPipeline(meshPipeline);

      commandList.bindDescriptor(meshPipeline, 0,
                                 mBindlessParams.at(frameIndex).getDescriptor(),
                                 offsets);

      uint32_t instanceStart = 0;
      for (auto &[handle, meshData] : frameData.getMeshGroups()) {
        const auto &mesh = mAssetRegistry.getMeshes().getAsset(handle).data;

        uint32_t numInstances =
            static_cast<uint32_t>(meshData.transforms.size());

        commandList.bindVertexBuffers(
            MeshRenderUtils::getGeometryBuffers(mesh),
            MeshRenderUtils::getGeometryBufferOffsets(mesh));
        commandList.bindIndexBuffer(mesh.indexBuffer, rhi::IndexType::Uint32);

        int32_t vertexOffset = 0;
        uint32_t indexOffset = 0;
        for (auto &geometry : mesh.geometries) {
          uint32_t indexCount = static_cast<uint32_t>(geometry.indices.size());
          int32_t vertexCount = static_cast<int32_t>(geometry.positions.size());

          commandList.drawIndexed(indexCount, indexOffset, vertexOffset,
                                  numInstances, instanceStart);
          vertexOffset += vertexCount;
          indexOffset += indexCount;
        }
        instanceStart += numInstances;
      }
    }

    // Skinned meshes
    {
      commandList.bindPipeline(skinnedMeshPipeline);

      commandList.bindDescriptor(skinnedMeshPipeline, 0,
                                 mBindlessParams.at(frameIndex).getDescriptor(),
                                 offsets);

      uint32_t instanceStart = 0;
      for (auto &[handle, meshData] : frameData.getSkinnedMeshGroups()) {
        const auto &mesh = mAssetRegistry.getMeshes().getAsset(handle).data;

        uint32_t numInstances =
            static_cast<uint32_t>(meshData.transforms.size());

        commandList.bindVertexBuffers(
            MeshRenderUtils::getSkinnedGeometryBuffers(mesh),
            MeshRenderUtils::getSkinnedGeometryBufferOffsets(mesh));
        commandList.bindIndexBuffer(mesh.indexBuffer, rhi::IndexType::Uint32);

        int32_t vertexOffset = 0;
        uint32_t indexOffset = 0;
        for (auto &geometry : mesh.geometries) {
          uint32_t indexCount = static_cast<uint32_t>(geometry.indices.size());
          int32_t vertexCount = static_cast<int32_t>(geometry.positions.size());

          commandList.drawIndexed(indexCount, indexOffset, vertexOffset,
                                  numInstances, instanceStart);
          vertexOffset += vertexCount;
          indexOffset += indexCount;
        }
        instanceStart += numInstances;
      }
    }

    // Text
    {
      commandList.bindPipeline(textPipeline);
      commandList.bindDescriptor(textPipeline, 0,
                                 mBindlessParams.at(frameIndex).getDescriptor(),
                                 offsets);

      static constexpr uint32_t NumVertices = 4;
      for (size_t i = 0; i < frameData.getTextEntities().size(); ++i) {
        commandList.pushConstants(
            textPipeline, rhi::ShaderStage::Vertex, 0, sizeof(glm::uvec4),
            glm::value_ptr(
                mMousePickingFrameData.at(frameIndex).textBounds.at(i)));

        commandList.draw(NumVertices, 0, 1, static_cast<uint32_t>(i));
      }
    }
  });

  for (auto &bp : mBindlessParams) {
    bp.build(mRenderStorage.getDevice());
  }
}

} // namespace quoll::editor