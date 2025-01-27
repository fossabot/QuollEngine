#pragma once

#include "quoll/animation/Animator.h"
#include "quoll/scene/PointLight.h"
#include "quoll/scene/DirectionalLight.h"
#include "quoll/renderer/MeshRenderer.h"
#include "quoll/renderer/SkinnedMeshRenderer.h"

namespace quoll {

/**
 * @brief Prefab component
 *
 * @tparam TValue value type
 */
template <class T> struct PrefabComponent {
  /**
   * Local entity ID
   *
   * This value is local to prefab asset file
   */
  u32 entity = 0;

  /**
   * Value
   */
  T value{};
};

/**
 * @brief Prefab transforms
 */
struct PrefabTransformData {
  /**
   * Local position
   */
  glm::vec3 position;

  /**
   * Local rotation
   */
  glm::quat rotation;

  /**
   * Local scale
   */
  glm::vec3 scale;

  /**
   * Parent ID
   *
   * This value is local to prefab asset file
   */
  i32 parent = -1;
};

/**
 * @brief Prefab asset data
 */
struct PrefabAsset {
  /**
   * List of transforms
   */
  std::vector<PrefabComponent<PrefabTransformData>> transforms;

  /**
   * List of meshes
   */
  std::vector<PrefabComponent<MeshAssetHandle>> meshes;

  /**
   * List of skeletons
   */
  std::vector<PrefabComponent<SkeletonAssetHandle>> skeletons;

  /**
   * List of animations
   */
  std::vector<AnimationAssetHandle> animations;

  /**
   * List of animators
   */
  std::vector<PrefabComponent<AnimatorAssetHandle>> animators;

  /**
   * List of point lights
   */
  std::vector<PrefabComponent<PointLight>> pointLights;

  /**
   * List of directional lights
   */
  std::vector<PrefabComponent<DirectionalLight>> directionalLights;

  /**
   * @brief List of entity names
   */
  std::vector<PrefabComponent<String>> names;

  /**
   * List of mesh renderers
   */
  std::vector<PrefabComponent<MeshRenderer>> meshRenderers;

  /**
   * List of skinned mesh renderers
   */
  std::vector<PrefabComponent<SkinnedMeshRenderer>> skinnedMeshRenderers;
};

} // namespace quoll
