#include "liquid/core/Base.h"
#include "liquid/core/Version.h"

#include "AssetCache.h"

#include "AssetFileHeader.h"
#include "OutputBinaryStream.h"
#include "InputBinaryStream.h"

#include "liquid/schemas/generated/Material.schema.h"

namespace liquid {

Result<Path>
AssetCache::createMaterialFromAsset(const AssetData<MaterialAsset> &asset) {
  flatbuffers::FlatBufferBuilder builder;

  auto baseColorTexture = builder.CreateString(getAssetRelativePath(
      mRegistry.getTextures(), asset.data.baseColorTexture));
  auto baseColorTextureCoord = asset.data.baseColorTextureCoord;
  auto baseColorFactor = schemas::base::Vec4(
      asset.data.baseColorFactor.x, asset.data.baseColorFactor.y,
      asset.data.baseColorFactor.z, asset.data.baseColorFactor.w);

  auto metallicRoughnessTexture = builder.CreateString(getAssetRelativePath(
      mRegistry.getTextures(), asset.data.metallicRoughnessTexture));
  auto metallicRoughnessTextureCoord = asset.data.metallicRoughnessTextureCoord;
  auto metallicFactor = asset.data.metallicFactor;
  auto roughnessFactor = asset.data.roughnessFactor;

  auto normalTexture = builder.CreateString(
      getAssetRelativePath(mRegistry.getTextures(), asset.data.normalTexture));
  auto normalTextureCoord = asset.data.normalTextureCoord;
  auto normalScale = asset.data.normalScale;

  auto occlusionTexture = builder.CreateString(getAssetRelativePath(
      mRegistry.getTextures(), asset.data.occlusionTexture));
  auto occlusionTextureCoord = asset.data.occlusionTextureCoord;
  auto occlusionStrength = asset.data.occlusionStrength;

  auto emissiveTexture = builder.CreateString(getAssetRelativePath(
      mRegistry.getTextures(), asset.data.emissiveTexture));
  auto emissiveTextureCoord = asset.data.emissiveTextureCoord;
  auto emissiveFactor = schemas::base::Vec3(asset.data.emissiveFactor.x,
                                            asset.data.emissiveFactor.y,
                                            asset.data.emissiveFactor.z);

  auto pbrMetallicRoughness = schemas::asset::CreatePBRMetallicRoughness(
      builder, baseColorTexture, baseColorTextureCoord, &baseColorFactor,
      metallicRoughnessTexture, metallicRoughnessTextureCoord, metallicFactor,
      roughnessFactor, normalTexture, normalTextureCoord, normalScale,
      occlusionTexture, occlusionTextureCoord, occlusionStrength,
      emissiveTexture, emissiveTextureCoord, &emissiveFactor);

  auto material = schemas::asset::CreateMaterial(
      builder, schemas::asset::MaterialData::MaterialData_PBRMetallicRoughness,
      pbrMetallicRoughness.Union());

  builder.Finish(material, schemas::asset::MaterialIdentifier());

  Path assetPath =
      (mAssetsPath / asset.name).replace_extension("material").make_preferred();

  const auto *ptr = builder.GetBufferPointer();
  std::ofstream stream(assetPath, std::ios::binary);
  stream.write(reinterpret_cast<const char *>(ptr), builder.GetSize());

  stream.close();

  return Result<Path>::Ok(assetPath);
}

Result<MaterialAssetHandle>
AssetCache::loadMaterialDataFromInputStream(std::istream &stream,
                                            const Path &filePath) {
  std::ifstream infile;
  infile.open(filePath, std::ios::binary);
  infile.seekg(0, std::ios::end);
  auto length = infile.tellg();
  infile.seekg(0, std::ios::beg);
  std::vector<uint8_t> buffer(length);
  infile.read(reinterpret_cast<char *>(buffer.data()), length);
  infile.close();

  flatbuffers::Verifier::Options options{};
  flatbuffers::Verifier verifier(buffer.data(), length, options);
  if (!schemas::asset::VerifyMaterialBuffer(verifier)) {
    return Result<MaterialAssetHandle>::Error("File is not a valid material: " +
                                              filePath.string());
  }

  auto *fbMaterial = schemas::asset::GetMaterial(buffer.data());

  if (!fbMaterial->Verify(verifier)) {
    return Result<MaterialAssetHandle>::Error("File is not a valid material: " +
                                              filePath.string());
  }

  auto *pbrMetallicRoughness = fbMaterial->data_as_PBRMetallicRoughness();

  if (!pbrMetallicRoughness->Verify(verifier)) {
    return Result<MaterialAssetHandle>::Error("File is not a valid material: " +
                                              filePath.string());
  }

  AssetData<MaterialAsset> material{};
  material.path = filePath;
  material.relativePath = std::filesystem::relative(filePath, mAssetsPath);
  material.name = material.relativePath.string();
  material.type = AssetType::Material;
  std::vector<String> warnings{};

  // Base color
  {
    const auto &res = getOrLoadTextureFromPath(
        pbrMetallicRoughness->base_color_texture()->string_view());

    if (res.hasData()) {
      material.data.baseColorTexture = res.getData();
      warnings.insert(warnings.end(), res.getWarnings().begin(),
                      res.getWarnings().end());
    } else {
      warnings.push_back(res.getError());
    }

    material.data.baseColorTextureCoord =
        pbrMetallicRoughness->base_color_texture_coordinate();

    material.data.baseColorFactor =
        glm::vec4{pbrMetallicRoughness->base_color_factor()->x(),
                  pbrMetallicRoughness->base_color_factor()->y(),
                  pbrMetallicRoughness->base_color_factor()->z(),
                  pbrMetallicRoughness->base_color_factor()->w()};
  }

  // Metallic roughness
  {
    const auto &res = getOrLoadTextureFromPath(
        pbrMetallicRoughness->metallic_roughness_texture()->string_view());

    if (res.hasData()) {
      material.data.metallicRoughnessTexture = res.getData();
      warnings.insert(warnings.end(), res.getWarnings().begin(),
                      res.getWarnings().end());
    } else {
      warnings.push_back(res.getError());
    }

    material.data.metallicRoughnessTextureCoord =
        pbrMetallicRoughness->metallic_roughness_texture_coordinate();

    material.data.metallicFactor = pbrMetallicRoughness->metallic_factor();
    material.data.roughnessFactor = pbrMetallicRoughness->roughness_factor();
  }

  // Normal
  {
    const auto &res = getOrLoadTextureFromPath(
        pbrMetallicRoughness->normal_texture()->string_view());

    if (res.hasData()) {
      material.data.normalTexture = res.getData();
      warnings.insert(warnings.end(), res.getWarnings().begin(),
                      res.getWarnings().end());
    } else {
      warnings.push_back(res.getError());
    }

    material.data.normalTextureCoord =
        pbrMetallicRoughness->normal_texture_coordinate();

    material.data.normalScale = pbrMetallicRoughness->normal_scale();
  }

  // Occlusion
  {
    const auto &res = getOrLoadTextureFromPath(
        pbrMetallicRoughness->occlusion_texture()->string_view());

    if (res.hasData()) {
      material.data.occlusionTexture = res.getData();
      warnings.insert(warnings.end(), res.getWarnings().begin(),
                      res.getWarnings().end());
    } else {
      warnings.push_back(res.getError());
    }

    material.data.occlusionTextureCoord =
        pbrMetallicRoughness->occlusion_texture_coordinate();

    material.data.occlusionStrength =
        pbrMetallicRoughness->occlusion_strength();
  }

  // Emissive
  {
    const auto &res = getOrLoadTextureFromPath(
        pbrMetallicRoughness->emissive_texture()->string_view());

    if (res.hasData()) {
      material.data.emissiveTexture = res.getData();
      warnings.insert(warnings.end(), res.getWarnings().begin(),
                      res.getWarnings().end());
    } else {
      warnings.push_back(res.getError());
    }

    material.data.emissiveTextureCoord =
        pbrMetallicRoughness->emissive_texture_coordinate();

    material.data.emissiveFactor =
        glm::vec3{pbrMetallicRoughness->emissive_factor()->x(),
                  pbrMetallicRoughness->emissive_factor()->y(),
                  pbrMetallicRoughness->emissive_factor()->z()};
  }

  return Result<MaterialAssetHandle>::Ok(
      mRegistry.getMaterials().addAsset(material), warnings);
}

Result<MaterialAssetHandle>
AssetCache::loadMaterialFromFile(const Path &filePath) {
  std::ifstream stream(filePath, std::ios::binary);

  if (!stream.good()) {
    return Result<MaterialAssetHandle>::Error(
        "File cannot be opened for reading: " + filePath.string());
  }

  return loadMaterialDataFromInputStream(stream, filePath);
}

Result<MaterialAssetHandle>
AssetCache::getOrLoadMaterialFromPath(StringView relativePath) {
  if (relativePath.empty()) {
    return Result<MaterialAssetHandle>::Ok(MaterialAssetHandle::Null);
  }

  Path fullPath = (mAssetsPath / relativePath).make_preferred();

  for (auto &[handle, asset] : mRegistry.getMaterials().getAssets()) {
    if (asset.path == fullPath) {
      return Result<MaterialAssetHandle>::Ok(handle);
    }
  }

  return loadMaterialFromFile(fullPath);
}

} // namespace liquid
