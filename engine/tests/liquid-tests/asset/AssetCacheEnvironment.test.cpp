#include "liquid/core/Base.h"
#include "liquid/core/Version.h"
#include "liquid/asset/AssetCache.h"
#include "liquid/asset/AssetFileHeader.h"
#include "liquid/asset/InputBinaryStream.h"
#include "liquid/schemas/generated/Environment.schema.h"

#include "liquid-tests/Testing.h"
#include "liquid-tests/schemas/generated/Test.schema.h"

static liquid::Path EnvDirectory = FixturesPath / "test-env";

class AssetCacheEnvironmentTest : public ::testing::Test {
public:
  AssetCacheEnvironmentTest() : cache(FixturesPath) {}

  void SetUp() override {
    std::filesystem::create_directory(EnvDirectory);
    std::filesystem::copy(FixturesPath / "1x1-cubemap.ktx",
                          EnvDirectory / "irradiance.ktx");
    std::filesystem::copy(FixturesPath / "1x1-cubemap.ktx",
                          EnvDirectory / "specular.ktx");
    std::filesystem::copy(FixturesPath / "1x1-2d.ktx",
                          EnvDirectory / "brdf.ktx");
  }

  void TearDown() override { std::filesystem::remove_all(EnvDirectory); }

public:
  liquid::AssetCache cache;
};

TEST_F(AssetCacheEnvironmentTest,
       CreatesEnvironmentWithTexturesNotLoadedAndLoadsTheEnvironmentFromFile) {
  auto irradianceMap =
      cache.loadTextureFromFile(EnvDirectory / "irradiance.ktx").getData();
  auto specularMap =
      cache.loadTextureFromFile(EnvDirectory / "specular.ktx").getData();

  liquid::AssetData<liquid::EnvironmentAsset> asset{};
  asset.name = "environment.lqenv";
  asset.path = FixturesPath / "environment.lqenv";
  asset.data.irradianceMap = irradianceMap;
  asset.data.specularMap = specularMap;
  auto createRes = cache.createEnvironmentFromAsset(asset);

  cache.getRegistry().getTextures().deleteAsset(irradianceMap);
  cache.getRegistry().getTextures().deleteAsset(specularMap);

  EXPECT_TRUE(cache.getRegistry().getTextures().getAssets().empty());

  auto res = cache.loadEnvironmentFromFile(createRes.getData());

  EXPECT_TRUE(res.hasData());
  EXPECT_FALSE(res.hasError());
  EXPECT_FALSE(res.hasWarnings());

  EXPECT_EQ(cache.getRegistry().getTextures().getAssets().size(), 2);
  EXPECT_FALSE(cache.getRegistry().getEnvironments().getAssets().empty());

  const auto &environment =
      cache.getRegistry().getEnvironments().getAsset(res.getData());
  EXPECT_NE(environment.data.irradianceMap, liquid::TextureAssetHandle::Null);
  EXPECT_NE(environment.data.specularMap, liquid::TextureAssetHandle::Null);
}

TEST_F(AssetCacheEnvironmentTest,
       CreatesEnvironmentWithTexturesLoadedAndLoadsTheEnvironmentFromFile) {
  auto irradianceMap =
      cache.loadTextureFromFile(EnvDirectory / "irradiance.ktx").getData();
  auto specularMap =
      cache.loadTextureFromFile(EnvDirectory / "specular.ktx").getData();

  liquid::AssetData<liquid::EnvironmentAsset> asset{};
  asset.name = "myenv";
  asset.data.irradianceMap = irradianceMap;
  asset.data.specularMap = specularMap;
  auto createRes = cache.createEnvironmentFromAsset(asset);

  EXPECT_EQ(cache.getRegistry().getTextures().getAssets().size(), 2);

  auto res = cache.loadEnvironmentFromFile(createRes.getData());

  EXPECT_TRUE(res.hasData());
  EXPECT_FALSE(res.hasError());
  EXPECT_FALSE(res.hasWarnings());

  EXPECT_EQ(cache.getRegistry().getTextures().getAssets().size(), 2);

  EXPECT_FALSE(cache.getRegistry().getEnvironments().getAssets().empty());

  const auto &environment =
      cache.getRegistry().getEnvironments().getAsset(res.getData());

  EXPECT_EQ(environment.data.irradianceMap, irradianceMap);
  EXPECT_EQ(environment.data.specularMap, specularMap);
}

TEST_F(AssetCacheEnvironmentTest,
       DoesNotLoadEnvironmentAssetIfEnvironmentTexturesCouldNotBeLoaded) {
  auto irradianceMap =
      cache.loadTextureFromFile(EnvDirectory / "irradiance.ktx").getData();
  auto specularMap =
      cache.loadTextureFromFile(EnvDirectory / "specular.ktx").getData();

  liquid::AssetData<liquid::EnvironmentAsset> asset{};
  asset.name = "environment.lqenv";
  asset.path = FixturesPath / "environment.lqenv";
  asset.data.irradianceMap = irradianceMap;
  asset.data.specularMap = specularMap;
  auto createRes = cache.createEnvironmentFromAsset(asset);

  cache.getRegistry().getTextures().deleteAsset(irradianceMap);
  cache.getRegistry().getTextures().deleteAsset(specularMap);

  {
    std::filesystem::remove(EnvDirectory / "irradiance.ktx");
    auto res = cache.loadEnvironmentFromFile(createRes.getData());

    EXPECT_TRUE(res.hasError());
    EXPECT_TRUE(cache.getRegistry().getTextures().getAssets().empty());
  }

  {
    std::filesystem::copy(FixturesPath / "1x1-cubemap.ktx",
                          EnvDirectory / "irradiance.ktx");
    std::filesystem::remove(EnvDirectory / "specular.ktx");
    auto res = cache.loadEnvironmentFromFile(createRes.getData());

    EXPECT_TRUE(res.hasError());
    EXPECT_TRUE(cache.getRegistry().getTextures().getAssets().empty());
  }
}

TEST_F(AssetCacheEnvironmentTest,
       FailsToLoadEnvironmentIfFileIdentifierMismatch) {
  flatbuffers::FlatBufferBuilder builder;

  auto testSchema = liquid::schemas::asset::CreateEnvironment(
      builder, builder.CreateString((EnvDirectory / "irradiance.ktx").string()),
      builder.CreateString((EnvDirectory / "specular.ktx").string()));
  builder.Finish(testSchema, "TEST");

  const auto *ptr = builder.GetBufferPointer();
  std::ofstream stream(FixturesPath / "invalid.environment", std::ios::binary);
  stream.write(reinterpret_cast<const char *>(ptr), builder.GetSize());
  stream.close();

  auto res = cache.loadAnimationFromFile(FixturesPath / "invalid.environment");
  EXPECT_TRUE(res.hasError());
}

TEST_F(AssetCacheEnvironmentTest, FailsToLoadAnimationIfInvalidSchema) {
  flatbuffers::FlatBufferBuilder builder;
  auto testSchema = liquid::schemas::test::CreateTest(
      builder, builder.CreateString("Test string"), 1);
  builder.Finish(testSchema, "LENV");

  const auto *ptr = builder.GetBufferPointer();
  std::ofstream stream(FixturesPath / "invalid.environment", std::ios::binary);
  stream.write(reinterpret_cast<const char *>(ptr), builder.GetSize());
  stream.close();

  auto res = cache.loadAnimationFromFile(FixturesPath / "invalid.environment");
  EXPECT_TRUE(res.hasError());
}
