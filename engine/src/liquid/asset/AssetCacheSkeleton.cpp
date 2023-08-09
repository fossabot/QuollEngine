#include "liquid/core/Base.h"
#include "liquid/core/Version.h"

#include "AssetCache.h"

#include "AssetFileHeader.h"
#include "OutputBinaryStream.h"
#include "InputBinaryStream.h"

#include "liquid/schemas/generated/Skeleton.schema.h"
#include "liquid/schemas/FlatbufferHelpers.h"

namespace liquid {

Result<Path>
AssetCache::createSkeletonFromAsset(const AssetData<SkeletonAsset> &asset) {
  const auto &data = asset.data;
  flatbuffers::FlatBufferBuilder builder;

  auto jointParents = builder.CreateVector(data.jointParents);
  auto jointPositions =
      builder.CreateVectorOfStructs(schemas::toFb(data.jointLocalPositions));
  auto jointRotations =
      builder.CreateVectorOfStructs(schemas::toFb(data.jointLocalRotations));
  auto jointScales =
      builder.CreateVectorOfStructs(schemas::toFb(data.jointLocalScales));
  auto jointInverseBindMatrices = builder.CreateVectorOfStructs(
      schemas::toFb(data.jointInverseBindMatrices));
  auto jointNames = builder.CreateVectorOfStrings(data.jointNames);

  auto skeleton = schemas::asset::CreateSkeleton(
      builder, jointParents, jointPositions, jointRotations, jointScales,
      jointInverseBindMatrices, jointNames);

  builder.Finish(skeleton, schemas::asset::SkeletonIdentifier());

  Path assetPath =
      (mAssetsPath / asset.name).replace_extension("skeleton").make_preferred();

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

Result<SkeletonAssetHandle>
AssetCache::loadSkeletonFromFile(const Path &filePath) {
  std::ifstream stream(filePath, std::ios::binary);
  if (!stream.good()) {
    return Result<SkeletonAssetHandle>::Error("Cannot open skeleton file: " +
                                              filePath.string());
  }

  stream.seekg(0, std::ios::end);
  auto length = stream.tellg();
  stream.seekg(0, std::ios::beg);
  std::vector<uint8_t> buffer(length);
  stream.read(reinterpret_cast<char *>(buffer.data()), length);
  stream.close();

  flatbuffers::Verifier::Options options{};
  flatbuffers::Verifier verifier(buffer.data(), length, options);
  if (!schemas::asset::VerifySkeletonBuffer(verifier)) {
    return Result<SkeletonAssetHandle>::Error("File is not a valid skeleton: " +
                                              filePath.string());
  }

  auto *fbSkeleton = schemas::asset::GetSkeleton(buffer.data());
  if (!fbSkeleton->Verify(verifier)) {
    return Result<SkeletonAssetHandle>::Error("File is not a valid skeleton: " +
                                              filePath.string());
  }

  auto numJoints = fbSkeleton->joint_parents()->size();

  bool jointListsMatch =
      numJoints > 0 && numJoints == fbSkeleton->joint_positions()->size() &&
      numJoints == fbSkeleton->joint_rotations()->size() &&
      numJoints == fbSkeleton->joint_scales()->size() &&
      numJoints == fbSkeleton->joint_inverse_bind_matrices()->size() &&
      numJoints == fbSkeleton->joint_names()->size();

  if (!jointListsMatch) {
    return Result<SkeletonAssetHandle>::Error(
        "Invalid number of joints in skeleton: " + filePath.string());
  }

  AssetData<SkeletonAsset> skeleton{};
  skeleton.path = filePath;
  skeleton.relativePath = std::filesystem::relative(filePath, mAssetsPath);
  skeleton.name = skeleton.relativePath.string();
  skeleton.type = AssetType::Skeleton;

  skeleton.data.jointParents = schemas::fromFb(fbSkeleton->joint_parents());
  skeleton.data.jointLocalPositions =
      schemas::fromFb(fbSkeleton->joint_positions());
  skeleton.data.jointLocalRotations =
      schemas::fromFb(fbSkeleton->joint_rotations());
  skeleton.data.jointLocalScales = schemas::fromFb(fbSkeleton->joint_scales());
  skeleton.data.jointInverseBindMatrices =
      schemas::fromFb(fbSkeleton->joint_inverse_bind_matrices());

  skeleton.data.jointNames = schemas::fromFb(fbSkeleton->joint_names());
  return Result<SkeletonAssetHandle>::Ok(
      mRegistry.getSkeletons().addAsset(skeleton));
}

Result<SkeletonAssetHandle>
AssetCache::getOrLoadSkeletonFromPath(StringView relativePath) {
  if (relativePath.empty()) {
    return Result<SkeletonAssetHandle>::Ok(SkeletonAssetHandle::Null);
  }

  Path fullPath = (mAssetsPath / relativePath).make_preferred();

  for (auto &[handle, asset] : mRegistry.getSkeletons().getAssets()) {
    if (asset.path == fullPath) {
      return Result<SkeletonAssetHandle>::Ok(handle);
    }
  }

  return loadSkeletonFromFile(fullPath);
}

} // namespace liquid
