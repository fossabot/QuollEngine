#include "liquid/core/Base.h"
#include <random>

#include "liquid/core/Version.h"
#include "liquid/asset/AssetCache.h"
#include "liquid/asset/AssetFileHeader.h"
#include "liquid/asset/InputBinaryStream.h"
#include "liquid/schemas/generated/Skeleton.schema.h"
#include "liquid/schemas/FlatbufferHelpers.h"

#include "liquid-tests/Testing.h"
#include "liquid-tests/schemas/generated/Test.schema.h"

class AssetCacheSkeletonTest : public ::testing::Test {
public:
  AssetCacheSkeletonTest() : cache(FixturesPath) {}

  liquid::AssetCache cache;
};

using AssetCacheDeathTest = AssetCacheSkeletonTest;

TEST_F(AssetCacheSkeletonTest, CreatesSkeletonAndLoadsItFromFile) {
  liquid::AssetData<liquid::SkeletonAsset> asset;
  asset.name = "test-skel0";
  {
    std::random_device device;
    std::mt19937 mt(device());
    std::uniform_real_distribution<float> dist(-5.0f, 10.0f);
    std::uniform_int_distribution<uint32_t> udist(0, 20);

    size_t countJoints = 20;
    for (size_t i = 0; i < countJoints; ++i) {
      asset.data.jointLocalPositions.push_back(
          glm::vec3(dist(mt), dist(mt), dist(mt)));
      asset.data.jointLocalRotations.push_back(
          glm::quat(dist(mt), dist(mt), dist(mt), dist(mt)));
      asset.data.jointLocalScales.push_back(
          glm::vec3(dist(mt), dist(mt), dist(mt)));
      asset.data.jointInverseBindMatrices.push_back(glm::mat4{
          // row 0
          dist(mt),
          dist(mt),
          dist(mt),
          dist(mt),
          // row 1
          dist(mt),
          dist(mt),
          dist(mt),
          dist(mt),

          // row 2
          dist(mt),
          dist(mt),
          dist(mt),
          dist(mt),

          // row3
          dist(mt),
          dist(mt),
          dist(mt),
          dist(mt),
      });

      asset.data.jointParents.push_back(udist(mt));
      asset.data.jointNames.push_back("Joint " + std::to_string(i));
    }
  }

  auto filePath = cache.createSkeletonFromAsset(asset);

  EXPECT_FALSE(filePath.hasError());
  EXPECT_TRUE(filePath.hasData());

  auto handle = cache.loadSkeletonFromFile(filePath.getData());

  EXPECT_FALSE(handle.hasError());
  EXPECT_TRUE(handle.hasData());

  EXPECT_NE(handle.getData(), liquid::SkeletonAssetHandle::Null);

  auto &actual = cache.getRegistry().getSkeletons().getAsset(handle.getData());

  EXPECT_EQ(actual.data.jointLocalPositions.size(),
            asset.data.jointLocalPositions.size());

  for (size_t i = 0; i < actual.data.jointLocalPositions.size(); ++i) {
    EXPECT_EQ(actual.data.jointLocalPositions.at(i),
              asset.data.jointLocalPositions.at(i));
    EXPECT_EQ(actual.data.jointLocalRotations.at(i),
              asset.data.jointLocalRotations.at(i));
    EXPECT_EQ(actual.data.jointLocalScales.at(i),
              asset.data.jointLocalScales.at(i));
    EXPECT_EQ(actual.data.jointParents.at(i), asset.data.jointParents.at(i));
    EXPECT_EQ(actual.data.jointInverseBindMatrices.at(i),
              asset.data.jointInverseBindMatrices.at(i));
    EXPECT_EQ(actual.data.jointNames.at(i), asset.data.jointNames.at(i));
  }
}

TEST_F(AssetCacheSkeletonTest, FailsToLoadSkeletonIfNoJoints) {
  flatbuffers::FlatBufferBuilder builder;

  auto testSchema = liquid::schemas::asset::CreateSkeleton(
      builder, builder.CreateVector(std::vector<uint8_t>(0)),
      builder.CreateVectorOfStructs(
          liquid::schemas::toFb(std::vector<glm::vec3>(0))),
      builder.CreateVectorOfStructs(
          liquid::schemas::toFb(std::vector<glm::quat>(0))),
      builder.CreateVectorOfStructs(
          liquid::schemas::toFb(std::vector<glm::vec3>(0))),
      builder.CreateVectorOfStructs(
          liquid::schemas::toFb(std::vector<glm::mat4>(0))),
      builder.CreateVectorOfStrings(std::vector<liquid::String>(0)));
  builder.Finish(testSchema, "LSKL");

  const auto *ptr = builder.GetBufferPointer();
  std::ofstream stream(FixturesPath / "invalid.skeleton", std::ios::binary);
  stream.write(reinterpret_cast<const char *>(ptr), builder.GetSize());
  stream.close();

  auto res = cache.loadSkeletonFromFile(FixturesPath / "invalid.skeleton");
  EXPECT_TRUE(res.hasError());
}

TEST_F(AssetCacheSkeletonTest, FailsToLoadSkeletonIfJointListsDoNotMatch) {
  flatbuffers::FlatBufferBuilder builder;

  auto testSchema = liquid::schemas::asset::CreateSkeleton(
      builder, builder.CreateVector(std::vector<uint8_t>(1)),
      builder.CreateVectorOfStructs(
          liquid::schemas::toFb(std::vector<glm::vec3>(0))),
      builder.CreateVectorOfStructs(
          liquid::schemas::toFb(std::vector<glm::quat>(0))),
      builder.CreateVectorOfStructs(
          liquid::schemas::toFb(std::vector<glm::vec3>(0))),
      builder.CreateVectorOfStructs(
          liquid::schemas::toFb(std::vector<glm::mat4>(0))),
      builder.CreateVectorOfStrings(std::vector<liquid::String>(0)));
  builder.Finish(testSchema, "LSKL");

  const auto *ptr = builder.GetBufferPointer();
  std::ofstream stream(FixturesPath / "invalid.skeleton", std::ios::binary);
  stream.write(reinterpret_cast<const char *>(ptr), builder.GetSize());
  stream.close();

  auto res = cache.loadSkeletonFromFile(FixturesPath / "invalid.skeleton");
  EXPECT_TRUE(res.hasError());
}

TEST_F(AssetCacheSkeletonTest, FailsToLoadSkeletonIfFileIdentifierMismatch) {
  flatbuffers::FlatBufferBuilder builder;

  auto testSchema = liquid::schemas::asset::CreateSkeleton(
      builder, builder.CreateVector(std::vector<uint8_t>(1)),
      builder.CreateVectorOfStructs(
          liquid::schemas::toFb(std::vector<glm::vec3>(1))),
      builder.CreateVectorOfStructs(
          liquid::schemas::toFb(std::vector<glm::quat>(1))),
      builder.CreateVectorOfStructs(
          liquid::schemas::toFb(std::vector<glm::vec3>(1))),
      builder.CreateVectorOfStructs(
          liquid::schemas::toFb(std::vector<glm::mat4>(1))),
      builder.CreateVectorOfStrings(std::vector<liquid::String>(1)));
  builder.Finish(testSchema, "TEST");

  const auto *ptr = builder.GetBufferPointer();
  std::ofstream stream(FixturesPath / "invalid.skeleton", std::ios::binary);
  stream.write(reinterpret_cast<const char *>(ptr), builder.GetSize());
  stream.close();

  auto res = cache.loadSkeletonFromFile(FixturesPath / "invalid.skeleton");
  EXPECT_TRUE(res.hasError());
}

TEST_F(AssetCacheSkeletonTest, FailsToLoadSkeletonIfInvalidSchema) {
  flatbuffers::FlatBufferBuilder builder;
  auto testSchema = liquid::schemas::test::CreateTest(
      builder, builder.CreateString("Test string"), 1);
  builder.Finish(testSchema, "LSKL");

  const auto *ptr = builder.GetBufferPointer();
  std::ofstream stream(FixturesPath / "invalid.skeleton", std::ios::binary);
  stream.write(reinterpret_cast<const char *>(ptr), builder.GetSize());
  stream.close();

  auto res = cache.loadSkeletonFromFile(FixturesPath / "invalid.skeleton");
  EXPECT_TRUE(res.hasError());
}
