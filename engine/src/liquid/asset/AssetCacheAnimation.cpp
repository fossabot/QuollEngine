#include "liquid/core/Base.h"
#include "liquid/core/Version.h"

#include "AssetCache.h"

#include "AssetFileHeader.h"
#include "OutputBinaryStream.h"
#include "InputBinaryStream.h"

#include "liquid/schemas/generated/Animation.schema.h"
#include "liquid/schemas/FlatbufferHelpers.h"

namespace liquid {

static schemas::asset::KeyframeInterpolation
keyframeInterpolationToFb(KeyframeSequenceAssetInterpolation interpolation) {
  if (interpolation == KeyframeSequenceAssetInterpolation::Linear) {
    return schemas::asset::KeyframeInterpolation_Linear;
  }

  return schemas::asset::KeyframeInterpolation_Step;
}

static schemas::asset::KeyframeValues
keyframeTargetToFb(KeyframeSequenceAssetTarget target) {
  if (target == KeyframeSequenceAssetTarget::Scale) {
    return schemas::asset::KeyframeValues_KeyframeScaleValues;
  }

  if (target == KeyframeSequenceAssetTarget::Rotation) {
    return schemas::asset::KeyframeValues_KeyframeRotationValues;
  }

  return schemas::asset::KeyframeValues_KeyframePositionValues;
}

static KeyframeSequenceAssetTarget
fbToKeyframeTarget(schemas::asset::KeyframeValues target) {
  if (target == schemas::asset::KeyframeValues_KeyframeScaleValues) {
    return KeyframeSequenceAssetTarget::Scale;
  }

  if (target == schemas::asset::KeyframeValues_KeyframeRotationValues) {
    return KeyframeSequenceAssetTarget::Rotation;
  }

  return KeyframeSequenceAssetTarget::Position;
}

static KeyframeSequenceAssetInterpolation
fbToKeyframeInterpolation(schemas::asset::KeyframeInterpolation interpolation) {
  if (interpolation == schemas::asset::KeyframeInterpolation_Linear) {
    return KeyframeSequenceAssetInterpolation::Linear;
  }

  return KeyframeSequenceAssetInterpolation::Step;
}

static flatbuffers::Offset<> valuesToFb(flatbuffers::FlatBufferBuilder &builder,
                                        const KeyframeSequenceAsset &keyframe) {
  if (keyframe.target == KeyframeSequenceAssetTarget::Position) {
    return schemas::asset::CreateKeyframePositionValues(
               builder, builder.CreateVectorOfStructs(
                            schemas::toFb(keyframe.keyframeValues)))
        .Union();
  }

  if (keyframe.target == KeyframeSequenceAssetTarget::Rotation) {
    return schemas::asset::CreateKeyframeRotationValues(
               builder, builder.CreateVectorOfStructs(
                            schemas::toFb(keyframe.keyframeValues)))
        .Union();
  }

  return schemas::asset::CreateKeyframePositionValues(
             builder, builder.CreateVectorOfStructs(
                          schemas::toFb(keyframe.keyframeValues)))
      .Union();
}

static std::vector<glm::vec4>
fbToValues(const schemas::asset::Keyframe *keyframe) {
  auto target = keyframe->values_type();
  if (target == schemas::asset::KeyframeValues_KeyframeScaleValues) {
    return schemas::fromFb(keyframe->values_as_KeyframeScaleValues()->data());
  }

  if (target == schemas::asset::KeyframeValues_KeyframeRotationValues) {
    return schemas::fromFb(
        keyframe->values_as_KeyframeRotationValues()->data());
  }

  if (target == schemas::asset::KeyframeValues_KeyframePositionValues) {
    return schemas::fromFb(
        keyframe->values_as_KeyframePositionValues()->data());
  }

  return {};
}

Result<Path>
AssetCache::createAnimationFromAsset(const AssetData<AnimationAsset> &asset) {
  flatbuffers::FlatBufferBuilder builder;

  std::vector<flatbuffers::Offset<schemas::asset::Keyframe>> keyframes;
  keyframes.reserve(asset.data.keyframes.size());

  for (const auto &keyframe : asset.data.keyframes) {
    keyframes.push_back(schemas::asset::CreateKeyframe(
        builder, builder.CreateVector(keyframe.keyframeTimes),
        keyframeTargetToFb(keyframe.target), valuesToFb(builder, keyframe),
        keyframeInterpolationToFb(keyframe.interpolation), keyframe.jointTarget,
        keyframe.joint));
  }

  auto animation = schemas::asset::CreateAnimation(
      builder, asset.data.time, builder.CreateVector(keyframes));

  builder.Finish(animation, schemas::asset::AnimationIdentifier());

  Path assetPath = (mAssetsPath / asset.name)
                       .replace_extension("animation")
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

Result<AnimationAssetHandle>
AssetCache::loadAnimationDataFromInputStream(InputBinaryStream &as,
                                             const Path &filePath) {
  std::ifstream stream(filePath, std::ios::binary);
  if (!stream.good()) {
    return Result<AnimationAssetHandle>::Error("Cannot open animation file: " +
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
  if (!schemas::asset::VerifyAnimationBuffer(verifier)) {
    return Result<AnimationAssetHandle>::Error(
        "File is not a valid animation: " + filePath.string());
  }

  auto *fbAnimation = schemas::asset::GetAnimation(buffer.data());
  if (!fbAnimation->Verify(verifier)) {
    return Result<AnimationAssetHandle>::Error(
        "File is not a valid animation: " + filePath.string());
  }

  AssetData<AnimationAsset> animation{};
  animation.path = filePath;
  animation.relativePath = std::filesystem::relative(filePath, mAssetsPath);
  animation.name = animation.relativePath.string();
  animation.type = AssetType::Animation;

  animation.data.time = fbAnimation->time();
  animation.data.keyframes.resize(fbAnimation->keyframes()->size());

  for (size_t i = 0; i < fbAnimation->keyframes()->size(); ++i) {
    const auto *keyframe =
        fbAnimation->keyframes()->Get(static_cast<flatbuffers::uoffset_t>(i));

    animation.data.keyframes.at(i).target =
        fbToKeyframeTarget(keyframe->values_type());
    animation.data.keyframes.at(i).interpolation =
        fbToKeyframeInterpolation(keyframe->interpolation());
    animation.data.keyframes.at(i).joint = keyframe->joint();
    animation.data.keyframes.at(i).jointTarget = keyframe->joint_target();
    animation.data.keyframes.at(i).keyframeTimes =
        schemas::fromFb(keyframe->times());
    animation.data.keyframes.at(i).keyframeValues = fbToValues(keyframe);
  }

  return Result<AnimationAssetHandle>::Ok(
      mRegistry.getAnimations().addAsset(animation));
}

Result<AnimationAssetHandle>
AssetCache::loadAnimationFromFile(const Path &filePath) {
  InputBinaryStream stream(filePath);

  return loadAnimationDataFromInputStream(stream, filePath);
}

Result<AnimationAssetHandle>
AssetCache::getOrLoadAnimationFromPath(StringView relativePath) {
  if (relativePath.empty()) {
    return Result<AnimationAssetHandle>::Ok(AnimationAssetHandle::Null);
  }

  Path fullPath = (mAssetsPath / relativePath).make_preferred();

  for (auto &[handle, asset] : mRegistry.getAnimations().getAssets()) {
    if (asset.path == fullPath) {
      return Result<AnimationAssetHandle>::Ok(handle);
    }
  }

  return loadAnimationFromFile(fullPath);
}

} // namespace liquid
