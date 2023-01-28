#pragma once

#include "liquid/asset/AssetCache.h"

#include "liquidator/asset/gltf/TinyGLTF.h"

namespace liquid::editor {

/**
 * @brief Mapping for GLTF index to
 * engine specific asset
 *
 * @tparam THandle Asset handle
 */
template <class THandle> struct GLTFToAsset {
  /**
   * GLTF index to asset handle map
   */
  std::map<size_t, THandle> map;
};

/**
 * @brief Transient skeleton data
 *
 * Used to store GLTF skins an joints with engine
 * specific skeleton handles
 */
struct SkeletonData {
  /**
   * GLTF joint to engine specific joint I
   */
  std::unordered_map<uint32_t, uint32_t> gltfToNormalizedJointMap;

  /**
   * Joints that are associated with skins
   */
  std::unordered_map<uint32_t, uint32_t> jointSkinMap;

  /**
   * Skin map
   */
  GLTFToAsset<SkeletonAssetHandle> skeletonMap;
};

/**
 * @brief Transient animation data
 *
 * Used to store GLTF animations with engine
 * specific animation handles
 */
struct AnimationData {
  /**
   * Node to animation map
   */
  std::map<uint32_t, std::vector<AnimationAssetHandle>> nodeAnimationMap;

  /**
   * Skin to animation map
   */
  std::map<uint32_t, std::vector<AnimationAssetHandle>> skinAnimationMap;
};

/**
 * @brief GLTF import data
 *
 * Stores all the information to perform the import
 */
struct GLTFImportData {
  /**
   * Asset cache
   */
  AssetCache &assetCache;

  /**
   * Target path
   */
  Path targetPath;

  /**
   * @brief GLTF model
   */
  const tinygltf::Model &model;

  /**
   * @brief Warnings
   */
  std::vector<String> warnings;

  /**
   * @brief Texture map
   */
  GLTFToAsset<TextureAssetHandle> textures;

  /**
   * @brief Material map
   */
  GLTFToAsset<MaterialAssetHandle> materials;

  /**
   * @brief Skeleton data
   */
  SkeletonData skeletons;

  /**
   * @brief Animation data
   */
  AnimationData animations;

  /**
   * @brief Mesh map
   */
  GLTFToAsset<MeshAssetHandle> meshes;

  /**
   * @brief Skinned mesh map
   */
  GLTFToAsset<SkinnedMeshAssetHandle> skinnedMeshes;

  /**
   * @brief Output path
   */
  Result<Path> outputPath = Result<Path>::Error("Empty");
};

} // namespace liquid::editor