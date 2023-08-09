#include "liquid/core/Base.h"
#include "liquid/core/Version.h"

#include "liquid/asset/AssetCache.h"
#include "liquid/asset/InputBinaryStream.h"
#include "liquid/asset/OutputBinaryStream.h"
#include "liquid/asset/AssetFileHeader.h"

#include "liquid/schemas/generated/Environment.schema.h"

namespace liquid {

Result<Path> AssetCache::createEnvironmentFromAsset(
    const AssetData<EnvironmentAsset> &asset) {
  auto irradianceMapPath = mRegistry.getTextures()
                               .getAsset(asset.data.irradianceMap)
                               .relativePath.string();
  std::replace(irradianceMapPath.begin(), irradianceMapPath.end(), '\\', '/');

  auto specularMapPath = mRegistry.getTextures()
                             .getAsset(asset.data.specularMap)
                             .relativePath.string();
  std::replace(specularMapPath.begin(), specularMapPath.end(), '\\', '/');

  flatbuffers::FlatBufferBuilder builder;
  auto environment = schemas::asset::CreateEnvironment(
      builder, builder.CreateString(irradianceMapPath),
      builder.CreateString(specularMapPath));

  builder.Finish(environment, schemas::asset::EnvironmentIdentifier());

  Path assetPath = (mAssetsPath / asset.name)
                       .replace_extension("environment")
                       .make_preferred();

  const auto *ptr = builder.GetBufferPointer();
  std::ofstream stream(assetPath, std::ios::binary);
  if (!stream.good()) {
    return Result<Path>::Error("File cannot be opened for writing: " +
                               assetPath.string());
  }

  stream.write(reinterpret_cast<const char *>(ptr), builder.GetSize());
  stream.close();

  return Result<Path>::Ok(assetPath);
}

Result<EnvironmentAssetHandle>
AssetCache::loadEnvironmentFromFile(const Path &filePath) {
  std::ifstream stream(filePath, std::ios::binary);
  if (!stream.good()) {
    return Result<EnvironmentAssetHandle>::Error(
        "Cannot open environment file: " + filePath.string());
  }

  stream.seekg(0, std::ios::end);
  auto length = stream.tellg();
  stream.seekg(0, std::ios::beg);
  std::vector<uint8_t> buffer(length);
  stream.read(reinterpret_cast<char *>(buffer.data()), length);
  stream.close();

  flatbuffers::Verifier::Options options{};
  flatbuffers::Verifier verifier(buffer.data(), length, options);
  if (!schemas::asset::VerifyEnvironmentBuffer(verifier)) {
    return Result<EnvironmentAssetHandle>::Error(
        "File is not a valid environment: " + filePath.string());
  }

  auto *fbEnvironment = schemas::asset::GetEnvironment(buffer.data());
  if (!fbEnvironment->Verify(verifier)) {
    return Result<EnvironmentAssetHandle>::Error(
        "File is not a valid environment as: " + filePath.string());
  }

  auto irradianceMapRes =
      getOrLoadTextureFromPath(fbEnvironment->irradiance_map()->str());
  if (irradianceMapRes.hasError()) {
    return Result<EnvironmentAssetHandle>::Error(irradianceMapRes.getError());
  }

  auto specularMapRes =
      getOrLoadTextureFromPath(fbEnvironment->specular_map()->str());
  if (specularMapRes.hasError()) {
    mRegistry.getTextures().deleteAsset(irradianceMapRes.getData());
    return Result<EnvironmentAssetHandle>::Error(specularMapRes.getError());
  }

  AssetData<EnvironmentAsset> environment{};
  environment.path = filePath;
  environment.relativePath = std::filesystem::relative(filePath, mAssetsPath);
  environment.name = environment.relativePath.string();
  environment.type = AssetType::Environment;
  environment.data.irradianceMap = irradianceMapRes.getData();
  environment.data.specularMap = specularMapRes.getData();

  auto environmentHandle = mRegistry.getEnvironments().addAsset(environment);

  return Result<EnvironmentAssetHandle>::Ok(environmentHandle);
}

} // namespace liquid
