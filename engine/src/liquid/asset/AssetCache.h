#pragma once

#include "Result.h"
#include "AssetRegistry.h"

namespace liquid {

class InputBinaryStream;

/**
 * @brief Asset cache
 *
 * Loads and creates engine optimized
 * assets for usage within engine
 * functionality
 */
class AssetCache {
public:
  /**
   * @brief Create asset cache
   *
   * @param assetsPath Assets path
   * @param createDefaultObjects Create default objects
   */
  AssetCache(const Path &assetsPath, bool createDefaultObjects = false);

  /**
   * @brief Create texture from asset
   *
   * Create engine specific texture
   * asset from any texture asset
   *
   * @param asset Texture asset
   * @return Path to new texture asset
   */
  Result<Path> createTextureFromAsset(const AssetData<TextureAsset> &asset);

  /**
   * @brief Load texture from file
   *
   * @param filePath Path to asset
   * @return Texture asset handle
   */
  Result<TextureAssetHandle> loadTextureFromFile(const Path &filePath);

  /**
   * @brief Load font from file
   *
   * @param filePath Path to asset
   * @return Font asset handle
   */
  Result<FontAssetHandle> loadFontFromFile(const Path &filePath);

  /**
   * @brief Create material from asset
   *
   * Create engine specific material asset
   * from material data
   *
   * @param asset Material asset
   * @return Path to new material asset
   */
  Result<Path> createMaterialFromAsset(const AssetData<MaterialAsset> &asset);

  /**
   * @brief Load material from file
   *
   * @param filePath Path to asset
   * @return Material asset handle
   */
  Result<MaterialAssetHandle> loadMaterialFromFile(const Path &filePath);

  /**
   * @brief Create mesh from asset
   *
   * Create engine specific mesh asset
   * from mesh data
   *
   * @param asset Mesh asset
   * @return Path to new mesh asset
   */
  Result<Path> createMeshFromAsset(const AssetData<MeshAsset> &asset);

  /**
   * @brief Load mesh from file
   *
   * @param filePath Path to asset
   * @return Mesh asset handle
   */
  Result<MeshAssetHandle> loadMeshFromFile(const Path &filePath);

  /**
   * @brief Create skinned mesh from asset
   *
   * Create engine specific skinned mesh asset
   * from mesh data
   *
   * @param asset Mesh asset
   * @return Path to new mesh asset
   */
  Result<Path>
  createSkinnedMeshFromAsset(const AssetData<SkinnedMeshAsset> &asset);

  /**
   * @brief Load skinned mesh from file
   *
   * @param filePath Path to asset
   * @return Skinned mesh asset handle
   */
  Result<SkinnedMeshAssetHandle> loadSkinnedMeshFromFile(const Path &filePath);

  /**
   * @brief Create skeleton from asset
   *
   * Create engine specific skeleton asset
   * from mesh data
   *
   * @param asset Skeleton asset
   * @return Path to new skeleton asset
   */
  Result<Path> createSkeletonFromAsset(const AssetData<SkeletonAsset> &asset);

  /**
   * @brief Load skeleton from file
   *
   * @param filePath Path to asset
   * @return Skeleton asset handle
   */
  Result<SkeletonAssetHandle> loadSkeletonFromFile(const Path &filePath);

  /**
   * @brief Create animation from asset
   *
   * Create engine specific animation asset
   * from animation data
   *
   * @param asset Animation asset
   * @return Path to new animation asset
   */
  Result<Path> createAnimationFromAsset(const AssetData<AnimationAsset> &asset);

  /**
   * @brief Load animation from file
   *
   * @param filePath Path to asset
   * @return Animation asset handle
   */
  Result<AnimationAssetHandle> loadAnimationFromFile(const Path &filePath);

  /**
   * @brief Create animator from asset
   *
   * Create engine specific animator asset
   * from animator data
   *
   * @param asset Animator asset
   * @return Path to new animator asset
   */
  Result<Path> createAnimatorFromAsset(const AssetData<AnimatorAsset> &asset);

  /**
   * @brief Load animator from file
   *
   * @param filePath Path to asset
   * @param handle Existing asset handle
   * @return Animator asset handle
   */
  Result<AnimatorAssetHandle>
  loadAnimatorFromFile(const Path &filePath,
                       AnimatorAssetHandle handle = AnimatorAssetHandle::Null);

  /**
   * @brief Load audio from file
   *
   * @param filePath Path to asset
   * @return Audio asset handle
   */
  Result<AudioAssetHandle> loadAudioFromFile(const Path &filePath);

  /**
   * @brief Create prefab from asset
   *
   * Create engine specific prefab asset
   * from prefab data
   *
   * @param asset Prefab asset
   * @return Path to new prefab asset
   */
  Result<Path> createPrefabFromAsset(const AssetData<PrefabAsset> &asset);

  /**
   * @brief Load prefab from file
   *
   * @param filePath Path to asset
   * @return Prefab asset handle
   */
  Result<PrefabAssetHandle> loadPrefabFromFile(const Path &filePath);

  /**
   * @brief Create environment from asset
   *
   * Create engine specific environment asset
   * from environment data
   *
   * @param asset Environment asset
   * @return Path to new environment asset
   */
  Result<Path>
  createEnvironmentFromAsset(const AssetData<EnvironmentAsset> &asset);

  /**
   * @brief Load environment from file
   *
   * @param filePath Path to asset
   * @return Environment asset handle
   */
  Result<EnvironmentAssetHandle> loadEnvironmentFromFile(const Path &filePath);

  /**
   * @brief Load Lua script from file
   *
   * @param filePath Path to asset
   * @param handle Lua script handle
   * @return Lua script handle
   */
  Result<LuaScriptAssetHandle> loadLuaScriptFromFile(
      const Path &filePath,
      LuaScriptAssetHandle handle = LuaScriptAssetHandle::Null);

  /**
   * @brief Get asset registry
   *
   * @return Asset registry
   */
  inline AssetRegistry &getRegistry() { return mRegistry; }

  /**
   * @brief Get assets path
   *
   * @return Assets path
   */
  inline const Path &getAssetsPath() const { return mAssetsPath; }

  /**
   * @brief Preload all assets in assets directory
   *
   * @param renderStorage Render storage
   * @return Preload result
   */
  Result<bool> preloadAssets(RenderStorage &renderStorage);

  /**
   * @brief Get asset name from path
   *
   * @param path Absolute path
   * @return Asset name
   */
  String getAssetNameFromPath(const Path &path);

  /**
   * @brief Load single asset
   *
   * Automatically identifies the asset type
   * and loads it to registry
   *
   * @param path Path to asset
   * @return Load result
   */
  Result<bool> loadAsset(const Path &path);

private:
  /**
   * @brief Get relative path of the asset
   *
   * @tparam TAssetMap Asset map type
   * @param map Asset map
   * @param handle Asset handle
   * @return Relative path of asset
   */
  template <class TAssetMap>
  String getAssetRelativePath(TAssetMap &map,
                              typename TAssetMap::Handle handle) {
    if (handle != TAssetMap::Handle::Null) {
      auto &asset = map.getAsset(handle);
      auto path = std::filesystem::relative(asset.path, mAssetsPath).string();
      std::replace(path.begin(), path.end(), '\\', '/');
      return path;
    }

    return String("");
  }

  /**
   * @brief Check asset file for validity
   *
   * Check if the binary stream did not fail,
   * the header has the correct magic word,
   * and the asset type is correct
   *
   * @return Result
   */
  Result<bool> checkAssetFile(InputBinaryStream &file, const Path &filePath,
                              AssetType assetType);

  /**
   * @brief Load single asset
   *
   * Automatically identifies the asset type
   * and loads it to registry
   *
   * @param path Path to asset
   * @param updateExisting Update if asset already exists
   * @return Load result
   */
  Result<bool> loadAsset(const Path &path, bool updateExisting);

private:
  /**
   * @brief Load mesh from input stream
   *
   * @param stream Input stream
   * @param filePath Path to asset
   * @return Mesh asset handle
   */
  Result<MeshAssetHandle> loadMeshDataFromInputStream(InputBinaryStream &stream,
                                                      const Path &filePath);

  /**
   * @brief Load skinned mesh from input stream
   *
   * @param stream Input stream
   * @param filePath Path to asset
   * @return Skinned mesh asset handle
   */
  Result<SkinnedMeshAssetHandle>
  loadSkinnedMeshDataFromInputStream(InputBinaryStream &stream,
                                     const Path &filePath);

  /**
   * @brief Load prefab from input stream
   *
   * @param stream Input stream
   * @param filePath Path to asset
   * @return Prefab asset handle
   */
  Result<PrefabAssetHandle>
  loadPrefabDataFromInputStream(InputBinaryStream &stream,
                                const Path &filePath);

private:
  /**
   * @brief Get or load texture from path
   *
   * @param relativePath Path to texture
   * @return Existing or newly loaded texture
   */
  Result<TextureAssetHandle> getOrLoadTextureFromPath(StringView relativePath);

  /**
   * @brief Get or load material from path
   *
   * @param relativePath Path to material
   * @return Existing or newly loaded material
   */
  Result<MaterialAssetHandle>
  getOrLoadMaterialFromPath(StringView relativePath);

  /**
   * @brief Get or load mesh from path
   *
   * @param relativePath Path to mesh
   * @return Existing or newly loaded mesh
   */
  Result<MeshAssetHandle> getOrLoadMeshFromPath(StringView relativePath);

  /**
   * @brief Get or load skinned mesh from path
   *
   * @param relativePath Path to skinned mesh
   * @return Existing or newly loaded skinned mesh
   */
  Result<SkinnedMeshAssetHandle>
  getOrLoadSkinnedMeshFromPath(StringView relativePath);

  /**
   * @brief Get or load skeleton from path
   *
   * @param relativePath Path to skeleton
   * @return Existing or newly loaded skeleton
   */
  Result<SkeletonAssetHandle>
  getOrLoadSkeletonFromPath(StringView relativePath);

  /**
   * @brief Get or load animation from path
   *
   * @param relativePath Path to animation
   * @return Existing or newly loaded animation
   */
  Result<AnimationAssetHandle>
  getOrLoadAnimationFromPath(StringView relativePath);

  /**
   * @brief Get or load animator from path
   *
   * @param relativePath Path to animator
   * @return Existing or newly loaded animator
   */
  Result<AnimatorAssetHandle>
  getOrLoadAnimatorFromPath(StringView relativePath);

private:
  AssetRegistry mRegistry;
  Path mAssetsPath;
};

} // namespace liquid
