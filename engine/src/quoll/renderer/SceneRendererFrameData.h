#pragma once

#include "quoll/rhi/RenderDevice.h"
#include "quoll/asset/MeshAsset.h"
#include "quoll/entity/Entity.h"
#include "quoll/renderer/Material.h"
#include "quoll/entity/EntityDatabase.h"
#include "quoll/renderer/BindlessDrawParameters.h"
#include "quoll/scene/CascadedShadowMap.h"
#include "quoll/scene/WorldTransform.h"
#include "quoll/scene/Camera.h"
#include "quoll/scene/PerspectiveLens.h"
#include "quoll/scene/DirectionalLight.h"
#include "quoll/scene/PointLight.h"

namespace quoll {

/**
 * @brief Scene renderer frame data
 *
 * Stores everything necessary to
 * render a frame
 */
class SceneRendererFrameData {
public:
  /**
   * Default reserved space for buffers
   */
  static constexpr usize DefaultReservedSpace = 10000;

  /**
   * Maximum number of joints
   */
  static constexpr usize MaxNumJoints = 32;

  /**
   * Maximum number of lights
   */
  static constexpr usize MaxNumLights = 256;

  /**
   * Maximum number of shadow maps
   */
  static constexpr usize MaxShadowMaps = 16;

  /**
   * @brief Directional light data
   */
  struct DirectionalLightData {
    /**
     * Light data
     *
     * First three values are direction
     * Last value is intensity
     */
    glm::vec4 data;

    /**
     * Light color
     */
    glm::vec4 color;

    /**
     * Shadow data
     *
     * First parameter indicates if shadows are enabled
     * Second parameter is the shadow index
     * Third parameter is the number of cascades
     *  which is used in cascaded shadow mapping
     */
    glm::uvec4 shadowData{0};
  };

  /**
   * @brief Point light data
   */
  struct PointLightData {
    /**
     * Light data
     *
     * First three values are position
     * Last value is direction
     */
    glm::vec4 data;

    /**
     * Light range
     *
     * vec4 is used for padding purposes
     */
    glm::vec4 range;

    /**
     * Light color
     */
    glm::vec4 color;
  };

  /**
   * @brief Shadow map data
   */
  struct ShadowMapData {
    /**
     * Shadow matrix generated from light
     */
    glm::mat4 shadowMatrix;

    /**
     * Shadow data
     *
     * First parameter indicates shadow split depth
     * Second parameter indicates if
     *  percentage closer filtering is enabled
     */
    glm::vec4 data;
  };

  /**
   * @brief Scene data
   */
  struct SceneData {
    /**
     * Environment lighting type
     */
    enum EnvironmentLighting { None = 0, Color = 1, Texture = 2 };

    /**
     * Light data
     *
     * First parameter is number of directional lights
     * Second parameter is number of point lights
     *
     * Fourth parameter is environment lighting type
     *   (0 = none, 1 = color, 2 = texture)
     */
    glm::uvec4 data{0};

    /**
     * Scene textures
     *
     * First parameter is IBL irradiance map
     * Second parameter is IBL specular map
     * Third parameter is IBL BRDF LUT
     * Fourth parameter is shadow map
     */
    glm::uvec4 textures{0};

    /**
     * Scene color
     */
    glm::vec4 color;
  };

  /**
   * @brief Skybox data
   */
  struct SkyboxData {
    /**
     * Skybox data
     *
     * First value represents texture id
     */
    glm::uvec4 data{0};

    /**
     * Skybox color
     */
    glm::vec4 color;
  };

  /**
   * @brief Material range
   */
  struct MaterialRange {
    /**
     * Range start
     */
    u32 start = 0;

    /**
     * Range end
     */
    u32 end = 0;
  };

  /**
   * @brief Mesh data
   */
  struct MeshData {

    /**
     * @brief Transforms of mesh entities
     */
    std::vector<glm::mat4> transforms;

    /**
     * @brief Mateiral ranges
     */
    std::vector<MaterialRange> materialRanges;

    /**
     * @brief Ids of mesh entities
     */
    std::vector<Entity> entities;
  };

  /**
   * @brief Skinned mesh data
   */
  struct SkinnedMeshData : public MeshData {
    /**
     * @brief Skeleton bone transforms
     */
    std::unique_ptr<glm::mat4> skeletons;

    /**
     * Last skeleton index
     */
    usize lastSkeleton = 0;

    /**
     * Skeleton capacity
     */
    usize skeletonCapacity = 0;
  };

  /**
   * @brief Glyph data
   *
   * Used for storing glyph
   * data in object buffers
   */
  struct GlyphData {
    /**
     * Atlas bounds
     */
    glm::vec4 bounds;

    /**
     * Plane bounds
     */
    glm::vec4 planeBounds;
  };

  /**
   * @brief Text item
   */
  struct TextItem {
    /**
     * Font handle
     */
    rhi::TextureHandle fontTexture = rhi::TextureHandle::Null;

    /**
     * Glyph start position in glyphs buffer
     */
    u32 glyphStart = 0;

    /**
     * Text length
     */
    u32 length = 0;
  };

public:
  /**
   * @brief Create frame data
   *
   * @param renderStorage Render Storage
   * @param reservedSpace Reserved space for buffer data
   */
  SceneRendererFrameData(RenderStorage &renderStorage,
                         usize reservedSpace = DefaultReservedSpace);

  /**
   * @brief Update storage buffers
   */
  void updateBuffers();

  /**
   * @brief Get sprite entities
   *
   * @return Sprite entities
   */
  inline const std::vector<Entity> &getSpriteEntities() const {
    return mSpriteEntities;
  }

  /**
   * @brief Get mesh groups
   *
   * @return Mesh groups
   */
  inline const std::unordered_map<MeshAssetHandle, MeshData> &
  getMeshGroups() const {
    return mMeshGroups;
  }

  /**
   * @brief Get skinned mesh groups
   *
   * @return Skinned mesh groups
   */
  inline const std::unordered_map<MeshAssetHandle, SkinnedMeshData> &
  getSkinnedMeshGroups() const {
    return mSkinnedMeshGroups;
  }

  /**
   * @brief Get texts
   *
   * @return Texts
   */
  inline const std::vector<TextItem> &getTexts() const { return mTexts; }

  /**
   * @brief Get text glyphs
   *
   * @return Text glyphs
   */
  inline const std::vector<GlyphData> &getTextGlyphs() const {
    return mTextGlyphs;
  }

  /**
   * @brief Get text entities
   *
   * @return Text entities
   */
  inline const std::vector<Entity> &getTextEntities() const {
    return mTextEntities;
  }

  /**
   * @brief Get number of lights
   *
   * @return Number of lights
   */
  inline u32 getNumLights() const { return mSceneData.data.x; }

  /**
   * @brief Get number shadow maps
   *
   * @return Number of shadow maps
   */
  inline const usize getNumShadowMaps() const { return mShadowMaps.size(); }

  /**
   * @brief Set default material
   *
   * @param material Default material
   */
  void setDefaultMaterial(rhi::DeviceAddress material);

  /**
   * @brief Add mesh data
   *
   * @param handle Mesh handle
   * @param entity Entity
   * @param transform Mesh world transform
   * @param materials Materials
   */
  void addMesh(MeshAssetHandle handle, quoll::Entity entity,
               const glm::mat4 &transform,
               const std::vector<rhi::DeviceAddress> &materials);

  /**
   * @brief Add skinned mesh data
   *
   * @param handle Skinned mesh handle
   * @param entity Entity
   * @param transform Skinned mesh world transform
   * @param skeleton Skeleton joint transforms
   * @param materials Materials
   */
  void addSkinnedMesh(MeshAssetHandle handle, Entity entity,
                      const glm::mat4 &transform,
                      const std::vector<glm::mat4> &skeleton,
                      const std::vector<rhi::DeviceAddress> &materials);

  /**
   * @brief Set BRDF lookup table
   *
   * @param brdfLut BRDF Lookup table
   */
  void setBrdfLookupTable(rhi::TextureHandle brdfLut);

  /**
   * @brief Add directional light
   *
   * @param light Directional light component
   */
  void addLight(const DirectionalLight &light);

  /**
   * @brief Add directional light with shadows
   *
   * @param light Directional light
   * @param shadowMap Cascaded shadow map
   */
  void addLight(const DirectionalLight &light,
                const CascadedShadowMap &shadowMap);

  /**
   * @brief Add point light
   *
   * @param light Point light component
   * @param transform World transform
   */
  void addLight(const PointLight &light, const WorldTransform &transform);

  /**
   * @brief Add sprite
   *
   * @param entity Entity
   * @param texture Texture handle
   * @param worldTransform World transform
   */
  void addSprite(Entity entity, rhi::TextureHandle texture,
                 const glm::mat4 &worldTransform);

  /**
   * @brief Add text
   *
   * @param entity Entity
   * @param fontTexture Font texture handle
   * @param glyphs Text glyphs
   * @param transform Text world transform
   */
  void addText(Entity entity, rhi::TextureHandle fontTexture,
               const std::vector<GlyphData> &glyphs,
               const glm::mat4 &transform);

  /**
   * @brief Set skybox texture
   *
   * @param skyboxTexture Skybox texture
   */
  void setSkyboxTexture(rhi::TextureHandle skyboxTexture);

  /**
   * @brief Set skybox color
   *
   * @param color Skybox color
   */
  void setSkyboxColor(const glm::vec4 &color);

  /**
   * @brief Set environment textures
   *
   * @param irradianceMap Irradiance map
   * @param specularMap Specular map
   */
  void setEnvironmentTextures(rhi::TextureHandle irradianceMap,
                              rhi::TextureHandle specularMap);

  /**
   * @brief Set environment color
   *
   * @param color Environment color
   */
  void setEnvironmentColor(const glm::vec4 &color);

  /**
   * @brief Set camera data
   *
   * @param camera Camera data
   * @param lens Camera lens data
   */
  void setCameraData(const Camera &camera, const PerspectiveLens &lens);

  /**
   * @brief Set shadow map texture
   *
   * @param shadowmap Shadoow map texture
   */
  void setShadowMapTexture(rhi::TextureHandle shadowmap);

  /**
   * @brief Clear intermediary buffers
   */
  void clear();

  /**
   * @brief Get reserved space
   *
   * @return Reserved space
   */
  inline usize getReservedSpace() const { return mReservedSpace; }

  /**
   * @brief Get bindless parameters
   *
   * @return Bindless parameters
   */
  inline BindlessDrawParameters &getBindlessParams() { return mBindlessParams; }

  /**
   * @brief Get bindless parameters
   *
   * @return Bindless parameters
   */
  inline const BindlessDrawParameters &getBindlessParams() const {
    return mBindlessParams;
  }

  /**
   * @brief Get sprite transforms buffer
   *
   * @return Sprite transforms buffer
   */
  inline rhi::DeviceAddress getSpriteTransformsBuffer() const {
    return mSpriteTransformsBuffer.getAddress();
  }

  /**
   * @brief Get sprite textures buffer
   *
   * @return Sprite textures buffer
   */
  inline rhi::DeviceAddress getSpriteTexturesBuffer() const {
    return mSpriteTexturesBuffer.getAddress();
  }

  /**
   * @brief Get flattened materials
   *
   * @return Flattened materials buffers
   */
  inline rhi::DeviceAddress getFlattenedMaterialsBuffer() const {
    return mFlatMaterialsBuffer.getAddress();
  }

  /**
   * @brief Get mesh transforms buffer
   *
   * @return Mesh transforms buffer
   */
  inline rhi::DeviceAddress getMeshTransformsBuffer() const {
    return mMeshTransformsBuffer.getAddress();
  }

  /**
   * @brief Get mesh materials buffer
   *
   * @return Mesh materials buffer
   */
  inline rhi::DeviceAddress getMeshMaterialsBuffer() const {
    return mMeshMaterialsBuffer.getAddress();
  }

  /**
   * @brief Get skinned mesh transforms buffer
   *
   * @return Skinned mesh transforms buffer
   */
  inline rhi::DeviceAddress getSkinnedMeshTransformsBuffer() const {
    return mSkinnedMeshTransformsBuffer.getAddress();
  }

  /**
   * @brief Get skinned mesh materials buffer
   *
   * @return Skinned mesh materials buffer
   */
  inline rhi::DeviceAddress getSkinnedMeshMaterialsBuffer() const {
    return mSkinnedMeshMaterialsBuffer.getAddress();
  }

  /**
   * @brief Get text transforms buffer
   *
   * @return Text transforms buffer
   */
  inline rhi::DeviceAddress getTextTransformsBuffer() const {
    return mTextTransformsBuffer.getAddress();
  }

  /**
   * @brief Get skeletons buffer
   *
   * @return Skeletons buffer
   */
  inline rhi::DeviceAddress getSkeletonsBuffer() const {
    return mSkeletonsBuffer.getAddress();
  }

  /**
   * @brief Get camera buffer
   *
   * @return Camera buffer
   */
  inline rhi::DeviceAddress getCameraBuffer() const {
    return mCameraBuffer.getAddress();
  }

  /**
   * @brief Get scene buffer
   *
   * @return Scene buffer
   */
  inline rhi::DeviceAddress getSceneBuffer() const {
    return mSceneBuffer.getAddress();
  }

  /**
   * @brief Get directional lights buffer
   *
   * @return Directional lights buffer
   */
  inline rhi::DeviceAddress getDirectionalLightsBuffer() const {
    return mDirectionalLightsBuffer.getAddress();
  }

  /**
   * @brief Get point lights buffer
   *
   * @return Point lights buffer
   */
  inline rhi::DeviceAddress getPointLightsBuffer() const {
    return mPointLightsBuffer.getAddress();
  }

  /**
   * @brief Get shadow maps buffer
   *
   * @return Shadow maps buffer
   */
  inline rhi::DeviceAddress getShadowMapsBuffer() const {
    return mShadowMapsBuffer.getAddress();
  }

  /**
   * @brief Get skybox buffer
   *
   * @return Skybox buffer
   */
  inline rhi::DeviceAddress getSkyboxBuffer() const {
    return mSkyboxBuffer.getAddress();
  }

  /**
   * @brief Get glyphs buffer
   *
   * @return Glyphs buffer
   */
  inline rhi::DeviceAddress getGlyphsBuffer() const {
    return mTextGlyphsBuffer.getAddress();
  }

private:
  /**
   * @brief Add cascaded shadow maps
   *
   * @param light Directional light
   * @param shadowMap Cascaded shadow map
   */
  void addCascadedShadowMaps(const DirectionalLight &light,
                             const CascadedShadowMap &shadowMap);

private:
  std::vector<DirectionalLightData> mDirectionalLights;
  std::vector<PointLightData> mPointLights;
  std::vector<ShadowMapData> mShadowMaps;
  SceneData mSceneData{};
  SkyboxData mSkyboxData{};
  Camera mCameraData;
  PerspectiveLens mCameraLens;

  usize mLastSkeleton = 0;

  std::vector<rhi::DeviceAddress> mFlatMaterials;
  rhi::Buffer mFlatMaterialsBuffer;

  rhi::Buffer mMeshTransformsBuffer;
  rhi::Buffer mSkinnedMeshTransformsBuffer;
  rhi::Buffer mSkeletonsBuffer;
  rhi::Buffer mMeshMaterialsBuffer;
  rhi::Buffer mSkinnedMeshMaterialsBuffer;
  std::unordered_map<MeshAssetHandle, MeshData> mMeshGroups;
  std::unordered_map<MeshAssetHandle, SkinnedMeshData> mSkinnedMeshGroups;

  rhi::Buffer mSceneBuffer;
  rhi::Buffer mDirectionalLightsBuffer;
  rhi::Buffer mPointLightsBuffer;
  rhi::Buffer mShadowMapsBuffer;
  rhi::Buffer mCameraBuffer;
  rhi::Buffer mSkyboxBuffer;

  std::vector<glm::mat4> mSpriteTransforms;
  std::vector<rhi::TextureHandle> mSpriteTextures;
  std::vector<Entity> mSpriteEntities;
  rhi::Buffer mSpriteTransformsBuffer;
  rhi::Buffer mSpriteTexturesBuffer;

  std::vector<TextItem> mTexts;
  std::vector<glm::mat4> mTextTransforms;
  std::vector<Entity> mTextEntities;
  std::vector<GlyphData> mTextGlyphs;

  rhi::Buffer mTextTransformsBuffer;
  rhi::Buffer mTextGlyphsBuffer;

  usize mReservedSpace = 0;

  BindlessDrawParameters mBindlessParams;
};

} // namespace quoll
