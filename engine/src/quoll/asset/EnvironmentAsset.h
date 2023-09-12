#pragma once

namespace quoll {

/**
 * @brief Environment asset
 */
struct EnvironmentAsset {
  /**
   * Irradiance map
   */
  TextureAssetHandle irradianceMap = TextureAssetHandle::Null;

  /**
   * Specular map
   */
  TextureAssetHandle specularMap = TextureAssetHandle::Null;
};

} // namespace quoll