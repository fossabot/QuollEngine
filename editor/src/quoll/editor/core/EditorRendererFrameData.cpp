#include "quoll/core/Base.h"
#include "EditorRendererFrameData.h"

#include "quoll/renderer/MeshRenderUtils.h"

namespace quoll::editor {

static constexpr usize MaxNumJoints = 32;

EditorRendererFrameData::EditorRendererFrameData(RenderStorage &renderStorage,
                                                 usize reservedSpace)
    : mReservedSpace(reservedSpace),
      mBindlessParams(renderStorage.getDevice()
                          ->getDeviceInformation()
                          .getLimits()
                          .minUniformBufferOffsetAlignment) {
  mSkeletonTransforms.reserve(mReservedSpace);
  mNumBones.reserve(mReservedSpace);
  mGizmoTransforms.reserve(mReservedSpace);
  mOutlineTransforms.reserve(mReservedSpace);
  mSkeletonVector.reset(new glm::mat4[mReservedSpace * MaxNumBones]);

  rhi::BufferDescription defaultDesc{};
  defaultDesc.usage = rhi::BufferUsage::Storage;
  defaultDesc.size = mReservedSpace * sizeof(glm::mat4);
  defaultDesc.mapped = true;

  mSkeletonTransformsBuffer = renderStorage.createBuffer(defaultDesc);

  {
    auto desc = defaultDesc;
    desc.size = mReservedSpace * MaxNumBones * sizeof(glm::mat4);
    desc.debugName = "Skeleton bone transforms";
    mSkeletonBoneTransformsBuffer = renderStorage.createBuffer(desc);
  }

  {
    auto desc = defaultDesc;
    desc.data = mGizmoTransforms.data();
    desc.debugName = "Gizmo transforms";
    mGizmoTransformsBuffer = renderStorage.createBuffer(desc);
  }

  {
    auto desc = defaultDesc;
    desc.usage = rhi::BufferUsage::Uniform;
    desc.size = sizeof(Camera);
    desc.debugName = "Camera";
    mCameraBuffer = renderStorage.createBuffer(desc);
  }

  {
    auto desc = defaultDesc;
    desc.usage = rhi::BufferUsage::Uniform;
    desc.size = sizeof(glm::uvec4);
    desc.debugName = "Editor grid";
    mEditorGridBuffer = renderStorage.createBuffer(desc);
  }

  {
    auto desc = defaultDesc;
    desc.size = sizeof(CollidableEntity);
    desc.usage = rhi::BufferUsage::Uniform;
    desc.debugName = "Collidable entity";
    mCollidableEntityBuffer = renderStorage.createBuffer(desc);
  }

  {
    auto desc = defaultDesc;
    desc.size = mReservedSpace * MaxNumJoints * sizeof(glm::mat4);
    desc.debugName = "Outline skeletons";
    mOutlineSkeletonsBuffer = renderStorage.createBuffer(desc);
  }

  {
    auto desc = defaultDesc;
    desc.debugName = "Outline transforms";
    mOutlineTransformsBuffer = renderStorage.createBuffer(desc);
  }

  {
    auto desc = defaultDesc;
    desc.debugName = "Outline text glyphs";
    mOutlineTextGlyphsBuffer = renderStorage.createBuffer(desc);
  }
}

void EditorRendererFrameData::addSkeleton(
    const glm::mat4 &worldTransform,
    const std::vector<glm::mat4> &boneTransforms) {
  mSkeletonTransforms.push_back(worldTransform);
  mNumBones.push_back(static_cast<u32>(boneTransforms.size()));

  auto *currentSkeleton = mSkeletonVector.get() + (mLastSkeleton * MaxNumBones);
  usize dataSize = std::min(boneTransforms.size(), MaxNumBones);
  memcpy(currentSkeleton, boneTransforms.data(), dataSize * sizeof(glm::mat4));

  mLastSkeleton++;
}

void EditorRendererFrameData::setActiveCamera(const Camera &camera) {
  mCameraData = camera;
}

void EditorRendererFrameData::addGizmo(rhi::TextureHandle icon,
                                       const glm::mat4 &worldTransform) {
  mGizmoTransforms.push_back(worldTransform);
  mGizmoCounts[icon]++;
}

void EditorRendererFrameData::addSpriteOutline(
    const glm::mat4 &worldTransform) {
  mOutlineTransforms.push_back(worldTransform);
  mOutlineSpriteEnd++;
}

void EditorRendererFrameData::addTextOutline(
    rhi::TextureHandle fontTexture,
    const std::vector<SceneRendererFrameData::GlyphData> &glyphs,
    const glm::mat4 &worldTransform) {

  mOutlineTransforms.push_back(worldTransform);

  SceneRendererFrameData::TextItem textData{};
  textData.fontTexture = fontTexture;
  textData.glyphStart = static_cast<u32>(mTextGlyphOutlines.size());
  textData.length = static_cast<u32>(glyphs.size());

  for (auto &glyph : glyphs) {
    mTextGlyphOutlines.push_back(glyph);
  }

  mTextOutlines.push_back(textData);
  mOutlineTextEnd++;
}

void EditorRendererFrameData::addMeshOutline(const MeshAsset &mesh,
                                             const glm::mat4 &worldTransform) {
  MeshOutline outline{};
  outline.indexBuffer = mesh.indexBuffer;
  {
    auto data = MeshRenderUtils::getGeometryBuffers(mesh);
    outline.vertexBuffers = std::vector(data.begin(), data.end());
  }
  {
    auto data = MeshRenderUtils::getGeometryBufferOffsets(mesh);
    outline.vertexBufferOffsets = std::vector(data.begin(), data.end());
  }

  outline.indexCounts.resize(mesh.geometries.size());
  outline.indexOffsets.resize(mesh.geometries.size());
  outline.vertexOffsets.resize(mesh.geometries.size());

  mOutlineTransforms.push_back(worldTransform);

  u32 lastIndexOffset = 0;
  u32 lastVertexOffset = 0;
  for (usize i = 0; i < outline.indexCounts.size(); ++i) {
    outline.indexCounts.at(i) =
        static_cast<u32>(mesh.geometries.at(i).indices.size());
    outline.indexOffsets.at(i) = static_cast<u32>(lastIndexOffset);
    outline.vertexOffsets.at(i) = lastVertexOffset;
    lastIndexOffset += outline.indexCounts.at(i);
    lastVertexOffset +=
        static_cast<u32>(mesh.geometries.at(i).positions.size());
  }

  mMeshOutlines.push_back(outline);
  mOutlineMeshEnd++;
}

void EditorRendererFrameData::addSkinnedMeshOutline(
    const MeshAsset &mesh, const std::vector<glm::mat4> &skeleton,
    const glm::mat4 &worldTransform) {
  MeshOutline outline{};
  outline.indexBuffer = mesh.indexBuffer;
  {
    auto data = MeshRenderUtils::getSkinnedGeometryBuffers(mesh);
    outline.vertexBuffers = std::vector(data.begin(), data.end());
  }
  {
    auto data = MeshRenderUtils::getSkinnedGeometryBufferOffsets(mesh);
    outline.vertexBufferOffsets = std::vector(data.begin(), data.end());
  }

  outline.indexCounts.resize(mesh.geometries.size());
  outline.indexOffsets.resize(mesh.geometries.size());
  outline.vertexOffsets.resize(mesh.geometries.size());

  mOutlineTransforms.push_back(worldTransform);

  u32 lastIndexOffset = 0;
  u32 lastVertexOffset = 0;
  for (usize i = 0; i < outline.indexCounts.size(); ++i) {
    outline.indexCounts.at(i) =
        static_cast<u32>(mesh.geometries.at(i).indices.size());
    outline.indexOffsets.at(i) = static_cast<u32>(lastIndexOffset);
    outline.vertexOffsets.at(i) = lastVertexOffset;
    lastIndexOffset += outline.indexCounts.at(i);
    lastVertexOffset +=
        static_cast<u32>(mesh.geometries.at(i).positions.size());
  }

  mMeshOutlines.push_back(outline);

  usize currentOffset = mLastOutlineSkeleton * MaxNumJoints;
  usize newSize = currentOffset + MaxNumJoints;

  // Resize skeletons if new skeleton does not fit
  if (mOutlineSkeletonCapacity < newSize) {
    mOutlineSkeletonCapacity = newSize * 2;
    auto *newVector = new glm::mat4[mOutlineSkeletonCapacity];

    memcpy(newVector, mOutlineSkeletons.get(),
           currentOffset * sizeof(glm::mat4));
    mOutlineSkeletons.reset(newVector);
  }

  auto *currentSkeleton = mOutlineSkeletons.get() + currentOffset;
  usize dataSize = std::min(skeleton.size(), MaxNumJoints);
  memcpy(currentSkeleton, skeleton.data(), dataSize * sizeof(glm::mat4));
  mLastOutlineSkeleton++;

  mOutlineSkinnedMeshEnd++;
}

void EditorRendererFrameData::setEditorGrid(const glm::uvec4 &data) {
  mEditorGridData = data;
}

void EditorRendererFrameData::updateBuffers() {
  mCameraBuffer.update(&mCameraData, sizeof(Camera));
  mEditorGridBuffer.update(&mEditorGridData, sizeof(glm::uvec4));

  if (!mSkeletonTransforms.empty()) {
    mSkeletonTransformsBuffer.update(mSkeletonTransforms.data(),
                                     mSkeletonTransforms.size() *
                                         sizeof(glm::mat4));
    mSkeletonBoneTransformsBuffer.update(
        mSkeletonVector.get(), mLastSkeleton * MaxNumBones * sizeof(glm::mat4));
  }

  mGizmoTransformsBuffer.update(mGizmoTransforms.data(),
                                mGizmoTransforms.size() * sizeof(glm::mat4));

  mOutlineTransformsBuffer.update(
      mOutlineTransforms.data(), mOutlineTransforms.size() * sizeof(glm::mat4));

  mOutlineTextGlyphsBuffer.update(
      mTextGlyphOutlines.data(),
      mTextGlyphOutlines.size() * sizeof(SceneRendererFrameData::GlyphData));

  if (mLastOutlineSkeleton > 0) {
    auto *skeletonsBuffer =
        static_cast<glm::mat4 *>(mOutlineSkeletonsBuffer.map());
    memcpy(skeletonsBuffer, mOutlineSkeletons.get(),
           mLastOutlineSkeleton * MaxNumJoints * sizeof(glm::mat4));

    mOutlineSkeletonsBuffer.unmap();
  }

  mCollidableEntityBuffer.update(&mCollidableEntityParams,
                                 sizeof(CollidableEntity));
}

void EditorRendererFrameData::clear() {
  mSkeletonTransforms.clear();
  mGizmoTransforms.clear();
  mNumBones.clear();
  mGizmoCounts.clear();
  mLastSkeleton = 0;

  mMeshOutlines.clear();
  mTextOutlines.clear();
  mTextGlyphOutlines.clear();
  mOutlineSpriteEnd = 0;
  mOutlineTextEnd = 0;
  mOutlineMeshEnd = 0;
  mOutlineSkinnedMeshEnd = 0;
  mOutlineTransforms.clear();
  mLastOutlineSkeleton = 0;

  mCollidableEntity = Entity::Null;
}

void EditorRendererFrameData::setCollidable(
    Entity entity, const Collidable &collidable,
    const WorldTransform &worldTransform) {
  mCollidableEntity = entity;
  mCollidableEntityParams.worldTransform = worldTransform.worldTransform;
  mCollidableEntityParams.center =
      glm::vec4(collidable.geometryDesc.center, 0.0f);
  mCollidableEntityParams.type.x =
      static_cast<u32>(collidable.geometryDesc.type);

  if (collidable.geometryDesc.type == PhysicsGeometryType::Box) {
    const auto &params =
        std::get<PhysicsGeometryBox>(collidable.geometryDesc.params);
    mCollidableEntityParams.params = glm::vec4(params.halfExtents, 0.0f);
  } else if (collidable.geometryDesc.type == PhysicsGeometryType::Sphere) {
    const auto &params =
        std::get<PhysicsGeometrySphere>(collidable.geometryDesc.params);
    mCollidableEntityParams.params = glm::vec4(params.radius);
  } else if (collidable.geometryDesc.type == PhysicsGeometryType::Capsule) {
    const auto &params =
        std::get<PhysicsGeometryCapsule>(collidable.geometryDesc.params);
    mCollidableEntityParams.params =
        glm::vec4(params.radius, params.halfHeight, 0.0f, 0.0f);
  }
}

void EditorRendererFrameData::createBindlessParamsRange() {
  struct EditorDrawParams {
    rhi::DeviceAddress gizmoTransforms;
    rhi::DeviceAddress skeletonTransforms;
    rhi::DeviceAddress debugSkeletons;
    rhi::DeviceAddress collidableParams;
    rhi::DeviceAddress camera;
    rhi::DeviceAddress gridData;
    rhi::DeviceAddress outlineTransforms;
    rhi::DeviceAddress outlineSkeletons;
    rhi::DeviceAddress outlineTextGlyphs;
  };

  mBindlessParams.addRange(EditorDrawParams{
      mGizmoTransformsBuffer.getAddress(),
      mSkeletonTransformsBuffer.getAddress(),
      mSkeletonBoneTransformsBuffer.getAddress(),
      mCollidableEntityBuffer.getAddress(), mCameraBuffer.getAddress(),
      mEditorGridBuffer.getAddress(), mOutlineTransformsBuffer.getAddress(),
      mOutlineSkeletonsBuffer.getAddress(),
      mOutlineTextGlyphsBuffer.getAddress()});
}

} // namespace quoll::editor
