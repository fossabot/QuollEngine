
#pragma once

#include "quoll/rhi/RenderHandle.h"
#include "quoll/rhi/RenderDevice.h"
#include "quoll/asset/MeshAsset.h"

#include "quoll/scene/Camera.h"
#include "quoll/physics/Collidable.h"

#include "quoll/renderer/RenderStorage.h"
#include "quoll/renderer/BindlessDrawParameters.h"
#include "quoll/renderer/SceneRendererFrameData.h"
#include "quoll/entity/EntityDatabase.h"

namespace quoll::editor {

/**
 * @brief Frame data for editor renderer
 *
 * Store data for each frame
 */
class EditorRendererFrameData {
public:
  /**
   * @brief Maximum number of debug bones
   */
  static constexpr usize MaxNumBones = 64;

  /**
   * @brief Default reserved space for buffers
   */
  static constexpr usize DefaultReservedSpace = 2000;

  /**
   * @brief Collidable entity data for buffers
   */
  struct CollidableEntity {
    /**
     * Entity world transform matrix
     */
    glm::mat4 worldTransform;

    /**
     * Entity type
     */
    glm::uvec4 type;

    /**
     * Collidable center
     */
    glm::vec4 center;

    /**
     * Collidable parameters
     *
     * Parameters differ between different
     * shape types
     */
    glm::vec4 params;
  };

  /**
   * @brief Mesh outline
   */
  struct MeshOutline {
    /**
     * Vertex buffer
     */
    std::vector<rhi::BufferHandle> vertexBuffers;

    /**
     * Vertex buffer binding offsets
     */
    std::vector<u64> vertexBufferOffsets;

    /**
     * Index buffer
     */
    rhi::BufferHandle indexBuffer;
    /**
     * Index buffer
     */
    std::vector<u32> indexCounts;
    /**
     * Index offsets
     */
    std::vector<u32> indexOffsets;
    /**
     * Vertex offsets
     */
    std::vector<u32> vertexOffsets;
  };

public:
  /**
   * @brief Create frame data
   *
   * @param renderStorage Render storage
   * @param reservedSpace Reserved space for buffer data
   */
  EditorRendererFrameData(RenderStorage &renderStorage,
                          usize reservedSpace = DefaultReservedSpace);

  /**
   * @brief Add skeleton
   *
   * @param worldTransform World transform matrix
   * @param boneTransforms Skeleton bone transforms
   */
  void addSkeleton(const glm::mat4 &worldTransform,
                   const std::vector<glm::mat4> &boneTransforms);

  /**
   * @brief Get skeleton world transforms buffer
   *
   * @return Skeleton world transforms buffer
   */
  inline rhi::BufferHandle getSkeletonTransforms() const {
    return mSkeletonTransformsBuffer.getHandle();
  };

  /**
   * @brief Get skeleton bones buffer
   *
   * @return Skeleton bones buffer
   */
  inline rhi::BufferHandle getSkeletonBoneTransforms() const {
    return mSkeletonBoneTransformsBuffer.getHandle();
  }

  /**
   * @brief Get number of bones
   *
   * @return Number of bones
   */
  inline const std::vector<u32> &getBoneCounts() const { return mNumBones; }

  /**
   * @brief Set active camera
   *
   * @param camera Active camera
   */
  void setActiveCamera(const Camera &camera);

  /**
   * @brief Set editor grid data
   *
   * @param data Editor grid data
   */
  void setEditorGrid(const glm::uvec4 &data);

  /**
   * @brief Get editor grid buffer
   *
   * @return Editor grid buffer
   */
  inline rhi::BufferHandle getEditorGridBuffer() const {
    return mEditorGridBuffer.getHandle();
  }

  /**
   * @brief Get active camera buffer
   *
   * @return Active camera buffer
   */
  inline rhi::BufferHandle getActiveCameraBuffer() {
    return mCameraBuffer.getHandle();
  }

  /**
   * @brief Add gizmo
   *
   * @param icon Gizmo icon
   * @param worldTransform World transform
   */
  void addGizmo(rhi::TextureHandle icon, const glm::mat4 &worldTransform);

  /**
   * @brief Add sprite outline
   *
   * @param worldTransform World transform
   */
  void addSpriteOutline(const glm::mat4 &worldTransform);

  /**
   * @brief Add text outline
   *
   * @param fontTexture Font texture
   * @param glyphs Text glyphs
   * @param worldTransform World transform
   */
  void
  addTextOutline(rhi::TextureHandle fontTexture,
                 const std::vector<SceneRendererFrameData::GlyphData> &glyphs,
                 const glm::mat4 &worldTransform);

  /**
   * @brief Add mesh outline
   *
   * @param mesh Mesh asset
   * @param worldTransform World transform
   */
  void addMeshOutline(const MeshAsset &mesh, const glm::mat4 &worldTransform);

  /**
   * @brief Add skinned mesh outline
   *
   * @param mesh Skinned mesh asset
   * @param skeleton Skeleton joints
   * @param worldTransform World transform
   */
  void addSkinnedMeshOutline(const MeshAsset &mesh,
                             const std::vector<glm::mat4> &skeleton,
                             const glm::mat4 &worldTransform);

  /**
   * @brief Get mesh outlines
   *
   * @return Mesh outlines
   */
  inline const std::vector<MeshOutline> &getMeshOutlines() const {
    return mMeshOutlines;
  }

  /**
   * @brief Get mesh outlines
   *
   * @return Mesh outlines
   */
  inline const std::vector<SceneRendererFrameData::TextItem> &
  getTextOutlines() const {
    return mTextOutlines;
  }

  /**
   * @brief Get last sprite index in outline data
   *
   * @return Last sprite index
   */
  inline usize getOutlineSpriteEnd() const { return mOutlineSpriteEnd; }

  /**
   * @brief Get last text index in outline data
   *
   * @return Last text index
   */
  inline usize getOutlineTextEnd() const { return mOutlineTextEnd; }

  /**
   * @brief Get last mesh index in outline data
   *
   * @return Last mesh index
   */
  inline usize getOutlineMeshEnd() const { return mOutlineMeshEnd; }

  /**
   * @brief Get last skinned mesh index in outline data
   *
   * @return Last skinned mesh index
   */
  inline usize getOutlineSkinnedMeshEnd() const {
    return mOutlineSkinnedMeshEnd;
  }

  /**
   * @brief Get buffer for gizmo world transforms
   *
   * @return Gizmo transforms
   */
  inline rhi::BufferHandle getGizmoTransformsBuffer() const {
    return mGizmoTransformsBuffer.getHandle();
  }

  /**
   * @brief Get gizmo counts
   *
   * @return Gizmo counts per icon
   */
  inline const std::unordered_map<rhi::TextureHandle, u32> &
  getGizmoCounts() const {
    return mGizmoCounts;
  }

  /**
   * @brief Update hardware buffer
   */
  void updateBuffers();

  /**
   * @brief Clear local buffer
   */
  void clear();

  /**
   * @brief Set collidable entity
   *
   * @param entity Entity
   * @param collidable Collidable component
   * @param worldTransform World transform
   */
  void setCollidable(Entity entity, const Collidable &collidable,
                     const WorldTransform &worldTransform);

  /**
   * @brief Get collidable parameters buffer
   *
   * @return Collidable parameters buffer handle
   */
  inline rhi::BufferHandle getCollidableParamsBuffer() const {
    return mCollidableEntityBuffer.getHandle();
  }

  /**
   * @brief Check if entity is set
   *
   * @retval true Collidable entity is set
   * @retval false Collidable entity is not set
   */
  inline bool isCollidableEntitySelected() const {
    return mCollidableEntity != Entity::Null;
  }

  /**
   * @brief Get collidable shape type
   *
   * @return Collidable shape type
   */
  inline PhysicsGeometryType getCollidableShapeType() const {
    return static_cast<PhysicsGeometryType>(mCollidableEntityParams.type.x);
  }

  /**
   * @brief Get bindless parameters
   *
   * @return Bindless parameters
   */
  inline BindlessDrawParameters &getBindlessParams() { return mBindlessParams; }

  /**
   * @brief Create bindless params range
   */
  void createBindlessParamsRange();

private:
  usize mReservedSpace = 0;

  // Outlines
  std::vector<glm::mat4> mOutlineTransforms;
  rhi::Buffer mOutlineTransformsBuffer;

  usize mOutlineSpriteEnd = 0;

  usize mOutlineTextEnd = 0;
  std::vector<SceneRendererFrameData::TextItem> mTextOutlines;
  std::vector<SceneRendererFrameData::GlyphData> mTextGlyphOutlines;
  rhi::Buffer mOutlineTextGlyphsBuffer;

  usize mOutlineMeshEnd = 0;
  std::vector<MeshOutline> mMeshOutlines;

  usize mOutlineSkinnedMeshEnd = 0;
  std::unique_ptr<glm::mat4> mOutlineSkeletons;
  usize mLastOutlineSkeleton = 0;
  usize mOutlineSkeletonCapacity = 0;
  rhi::Buffer mOutlineSkeletonsBuffer;

  // Camera
  Camera mCameraData;
  rhi::Buffer mCameraBuffer;

  // Editor grid
  glm::uvec4 mEditorGridData{};
  rhi::Buffer mEditorGridBuffer;

  // Skeleton bones
  usize mLastSkeleton = 0;
  std::vector<glm::mat4> mSkeletonTransforms;
  std::unique_ptr<glm::mat4> mSkeletonVector;
  std::vector<u32> mNumBones;
  rhi::Buffer mSkeletonTransformsBuffer;
  rhi::Buffer mSkeletonBoneTransformsBuffer;

  // Gizmos
  std::vector<glm::mat4> mGizmoTransforms;
  std::unordered_map<rhi::TextureHandle, u32> mGizmoCounts;
  rhi::Buffer mGizmoTransformsBuffer;

  // Collidable shape
  Entity mCollidableEntity = Entity::Null;
  CollidableEntity mCollidableEntityParams{};

  rhi::Buffer mCollidableEntityBuffer;

  BindlessDrawParameters mBindlessParams;
};

} // namespace quoll::editor
