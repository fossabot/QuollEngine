#include "liquid/core/Base.h"
#include <random>

#include "liquid/core/Version.h"
#include "liquid/asset/AssetCache.h"
#include "liquid/asset/AssetFileHeader.h"
#include "liquid/schemas/generated/Animation.schema.h"

#include "liquid-tests/Testing.h"
#include "liquid-tests/schemas/generated/Test.schema.h"

class AssetCacheAnimationTest : public ::testing::Test {
public:
  AssetCacheAnimationTest() : cache(FixturesPath) {}

  liquid::AssetCache cache;
};

liquid::AssetData<liquid::AnimationAsset> createRandomizedAnimation() {
  liquid::AssetData<liquid::AnimationAsset> asset;
  asset.name = "test-anim0";
  {
    std::random_device device;
    std::mt19937 mt(device());
    std::uniform_real_distribution<float> dist(-5.0f, 10.0f);
    std::uniform_int_distribution<uint32_t> udist(0, 20);
    std::uniform_int_distribution<uint32_t> targetDist(0, 2);
    std::uniform_int_distribution<uint32_t> interpolationDist(0, 1);

    size_t countKeyframes = 5;
    size_t countKeyframeValues = 10;
    asset.data.time = static_cast<float>(countKeyframeValues) * 0.5f;

    for (size_t i = 0; i < countKeyframes; ++i) {
      liquid::KeyframeSequenceAsset keyframe;
      keyframe.interpolation =
          static_cast<liquid::KeyframeSequenceAssetInterpolation>(
              interpolationDist(mt));
      keyframe.target =
          static_cast<liquid::KeyframeSequenceAssetTarget>(targetDist(mt));
      keyframe.joint = udist(mt);
      keyframe.jointTarget = keyframe.joint == 10;
      keyframe.keyframeTimes.resize(countKeyframeValues);
      keyframe.keyframeValues.resize(countKeyframeValues);

      for (size_t j = 0; j < countKeyframeValues; ++j) {
        keyframe.keyframeTimes.at(j) = 0.5f * static_cast<float>(j);
        keyframe.keyframeValues.at(j) =
            glm::vec4(dist(mt), dist(mt), dist(mt), dist(mt));
      }

      asset.data.keyframes.push_back(keyframe);
    }
  }

  return asset;
}

TEST_F(AssetCacheAnimationTest, CreatesAnimationAndLoadsItFromFile) {
  auto asset = createRandomizedAnimation();

  auto filePath = cache.createAnimationFromAsset(asset);
  auto handle = cache.loadAnimationFromFile(filePath.getData());
  EXPECT_FALSE(handle.hasError());
  EXPECT_TRUE(handle.hasData());
  EXPECT_NE(handle.getData(), liquid::AnimationAssetHandle::Null);

  auto &actual = cache.getRegistry().getAnimations().getAsset(handle.getData());
  EXPECT_EQ(actual.type, liquid::AssetType::Animation);

  EXPECT_EQ(actual.data.time, asset.data.time);
  EXPECT_EQ(actual.data.keyframes.size(), asset.data.keyframes.size());
  for (size_t i = 0; i < asset.data.keyframes.size(); ++i) {
    auto &expectedKf = asset.data.keyframes.at(i);
    auto &actualKf = actual.data.keyframes.at(i);

    EXPECT_EQ(expectedKf.target, actualKf.target);
    EXPECT_EQ(expectedKf.interpolation, actualKf.interpolation);
    EXPECT_EQ(expectedKf.jointTarget, actualKf.jointTarget);
    EXPECT_EQ(expectedKf.joint, actualKf.joint);
    EXPECT_EQ(expectedKf.keyframeTimes.size(), actualKf.keyframeTimes.size());
    EXPECT_EQ(expectedKf.keyframeValues.size(), actualKf.keyframeValues.size());

    for (size_t j = 0; j < expectedKf.keyframeTimes.size(); ++j) {
      EXPECT_EQ(expectedKf.keyframeTimes.at(j), actualKf.keyframeTimes.at(j));
      EXPECT_EQ(expectedKf.keyframeValues.at(j), actualKf.keyframeValues.at(j));
    }
  }
}

TEST_F(AssetCacheAnimationTest, FailsToLoadAnimationIfFileIdentifierMismatch) {
  flatbuffers::FlatBufferBuilder builder;

  auto keyframes =
      builder
          .CreateVector<flatbuffers::Offset<liquid::schemas::asset::Keyframe>>(
              {});

  auto testSchema =
      liquid::schemas::asset::CreateAnimation(builder, 0.0f, keyframes);
  builder.Finish(testSchema, "TEST");

  const auto *ptr = builder.GetBufferPointer();
  std::ofstream stream(FixturesPath / "invalid.animation", std::ios::binary);
  stream.write(reinterpret_cast<const char *>(ptr), builder.GetSize());
  stream.close();

  auto res = cache.loadAnimationFromFile(FixturesPath / "invalid.animation");
  EXPECT_TRUE(res.hasError());
}

TEST_F(AssetCacheAnimationTest, FailsToLoadAnimationIfInvalidSchema) {
  flatbuffers::FlatBufferBuilder builder;
  auto testSchema = liquid::schemas::test::CreateTest(
      builder, builder.CreateString("Test string"), 1);
  builder.Finish(testSchema, "LANM");

  const auto *ptr = builder.GetBufferPointer();
  std::ofstream stream(FixturesPath / "invalid.animation", std::ios::binary);
  stream.write(reinterpret_cast<const char *>(ptr), builder.GetSize());
  stream.close();

  auto res = cache.loadAnimationFromFile(FixturesPath / "invalid.animation");
  EXPECT_TRUE(res.hasError());
}
