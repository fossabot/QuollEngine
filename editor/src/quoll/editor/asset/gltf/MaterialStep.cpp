#include "quoll/core/Base.h"

#include "MaterialStep.h"
#include "TextureUtils.h"

namespace quoll::editor {

/**
 * @brief Load materials into registry
 *
 * @param importData GLTF import data
 */
void loadMaterials(GLTFImportData &importData) {
  auto &assetCache = importData.assetCache;
  const auto &model = importData.model;
  const auto &textures = importData.textures;

  for (usize i = 0; i < model.materials.size(); ++i) {
    auto &gltfMaterial = model.materials.at(i);

    auto assetName = gltfMaterial.name.empty() ? "material" + std::to_string(i)
                                               : gltfMaterial.name;
    assetName += ".mat";

    AssetData<MaterialAsset> material;

    material.name = getGLTFAssetName(importData, assetName);
    material.uuid = getOrCreateGLTFUuid(importData, assetName);
    material.type = AssetType::Material;

    if (gltfMaterial.pbrMetallicRoughness.baseColorTexture.index >= 0) {
      material.data.baseColorTexture = loadTexture(
          importData, gltfMaterial.pbrMetallicRoughness.baseColorTexture.index,
          GLTFTextureColorSpace::Srgb, true);
    }
    material.data.baseColorTextureCoord = static_cast<i8>(
        gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord);
    auto &colorFactor = gltfMaterial.pbrMetallicRoughness.baseColorFactor;
    material.data.baseColorFactor = glm::vec4{colorFactor[0], colorFactor[1],
                                              colorFactor[2], colorFactor[3]};

    if (gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
      material.data.metallicRoughnessTexture = loadTexture(
          importData,
          gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index,
          GLTFTextureColorSpace::Linear, false);
    }
    material.data.metallicRoughnessTextureCoord = static_cast<i8>(
        gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord);
    material.data.metallicFactor =
        static_cast<f32>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
    material.data.roughnessFactor =
        static_cast<f32>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);

    if (gltfMaterial.normalTexture.index >= 0) {
      material.data.normalTexture =
          loadTexture(importData, gltfMaterial.normalTexture.index,
                      GLTFTextureColorSpace::Linear, false);
    }
    material.data.normalTextureCoord =
        static_cast<i8>(gltfMaterial.normalTexture.texCoord);
    material.data.normalScale =
        static_cast<f32>(gltfMaterial.normalTexture.scale);

    if (gltfMaterial.occlusionTexture.index >= 0) {
      material.data.occlusionTexture =
          loadTexture(importData, gltfMaterial.occlusionTexture.index,
                      GLTFTextureColorSpace::Linear, false);
    }
    material.data.occlusionTextureCoord =
        static_cast<i8>(gltfMaterial.occlusionTexture.texCoord);
    material.data.occlusionStrength =
        static_cast<f32>(gltfMaterial.occlusionTexture.strength);

    if (gltfMaterial.emissiveTexture.index >= 0) {
      material.data.emissiveTexture =
          loadTexture(importData, gltfMaterial.emissiveTexture.index,
                      GLTFTextureColorSpace::Srgb, false);
    }
    material.data.emissiveTextureCoord =
        static_cast<i8>(gltfMaterial.emissiveTexture.texCoord);
    auto &emissiveFactor = gltfMaterial.emissiveFactor;
    material.data.emissiveFactor =
        glm::vec3{emissiveFactor[0], emissiveFactor[1], emissiveFactor[2]};

    auto emissiveStrengthExt =
        gltfMaterial.extensions.find("KHR_materials_emissive_strength");

    if (emissiveStrengthExt != gltfMaterial.extensions.end() &&
        emissiveStrengthExt->second.IsObject()) {
      auto strength = emissiveStrengthExt->second.Get("emissiveStrength");
      if (strength.IsReal()) {
        material.data.emissiveFactor *=
            static_cast<f32>(strength.GetNumberAsDouble());
      }
    }

    auto path = assetCache.createMaterialFromAsset(material);
    auto handle = assetCache.loadMaterial(material.uuid);
    importData.materials.map.insert_or_assign(i, handle.getData());

    importData.outputUuids.insert_or_assign(assetName,
                                            assetCache.getRegistry()
                                                .getMaterials()
                                                .getAsset(handle.getData())
                                                .uuid);
  }
}

} // namespace quoll::editor
