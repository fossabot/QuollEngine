#include "quoll/core/Base.h"
#include "quoll/core/Engine.h"
#include "quoll/yaml/Yaml.h"
#include "quoll/rhi-mock/MockRenderDevice.h"

#include "quoll/editor-tests/Testing.h"
#include "quoll/editor/asset/AssetManager.h"

#include <cryptopp/sha.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>

namespace fs = std::filesystem;

static const quoll::Path AssetsPath = fs::current_path() / "assets";
static const quoll::Path CachePath = fs::current_path() / "cache";
static const quoll::Path FixturesPath = fs::current_path() / "fixtures";
static const quoll::Path TempPath = fs::current_path() / "temp";
static const quoll::Path InnerPathInAssets = AssetsPath / "inner-1" / "inner-2";

class AssetManagerTest : public ::testing::Test {
public:
  AssetManagerTest()
      : renderStorage(&device),
        manager(AssetsPath, CachePath, renderStorage, false, false) {}

  void SetUp() override {
    fs::create_directory(AssetsPath);
    fs::create_directory(CachePath);
    fs::create_directory(TempPath);
  }

  void TearDown() override {
    fs::remove_all(AssetsPath);
    fs::remove_all(CachePath);
    fs::remove_all(TempPath);
  }

  void createEmptyFile(quoll::Path path) {
    std::ofstream stream(path);
    stream.close();
  }

public:
  quoll::rhi::MockRenderDevice device;
  quoll::RenderStorage renderStorage;
  quoll::editor::AssetManager manager;
};

class AssetTest : public AssetManagerTest,
                  public ::testing::WithParamInterface<
                      std::tuple<quoll::String, quoll::String>> {};

template <class T>
std::vector<std::tuple<quoll::String, quoll::String>>
mapExtensions(const std::vector<quoll::String> &extensions, T &&fn) {
  std::vector<std::tuple<quoll::String, quoll::String>> temp(extensions.size());
  std::transform(extensions.begin(), extensions.end(), temp.begin(),
                 [&fn](auto str) {
                   return std::tuple{str, fn(str)};
                 });
  return temp;
}

#define InitAssetTestSuite(SuiteName, Extensions, Fn)                          \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      SuiteName, AssetTest, testing::ValuesIn(mapExtensions(Extensions, Fn)),  \
      [](const ::testing::TestParamInfo<AssetTest::ParamType> &info) {         \
        return std::get<0>(info.param);                                        \
      })

TEST_F(AssetManagerTest, SetsProvidedAssetsAndCachePathsOnConstruct) {
  EXPECT_EQ(manager.getAssetsPath(),
            std::filesystem::current_path() / "assets");
  EXPECT_EQ(manager.getCachePath(), std::filesystem::current_path() / "cache");
}

TEST_F(AssetManagerTest, CreatesScriptFileAndLoadsIt) {
  auto handle = manager.createLuaScript(InnerPathInAssets / "test");

  EXPECT_TRUE(handle.hasData());
  EXPECT_EQ(handle.getData(), InnerPathInAssets / "test.lua");
  EXPECT_TRUE(fs::exists(InnerPathInAssets / ("test.lua")));
}

TEST_F(AssetManagerTest, CreatesAnimatorFileAndLoadsIt) {
  auto handle = manager.createAnimator(InnerPathInAssets / "test");

  EXPECT_TRUE(handle.hasData());
  EXPECT_EQ(handle.getData(), InnerPathInAssets / "test.animator");
  EXPECT_TRUE(fs::exists(InnerPathInAssets / "test.animator"));
}

TEST_F(AssetManagerTest, CreatesInputMapFileAndLoadsIt) {
  auto handle = manager.createInputMap(InnerPathInAssets / "test");

  EXPECT_TRUE(handle.hasData());
  EXPECT_EQ(handle.getData(), InnerPathInAssets / "test.inputmap");
  EXPECT_TRUE(fs::exists(InnerPathInAssets / "test.inputmap"));
}

TEST_F(AssetManagerTest, ReloadingAssetIfChangedDoesNotCreateFileWithNewUUID) {
  fs::create_directories(InnerPathInAssets);

  auto animatorPath = InnerPathInAssets / "test.animator";
  auto metaPath = animatorPath;
  metaPath.replace_extension("animator.meta");

  auto sourcePath = manager.createAnimator(animatorPath).getData();
  auto engineUuidBefore = manager.findRootAssetUuid(sourcePath);
  EXPECT_TRUE(engineUuidBefore.isValid());

  std::filesystem::remove(
      (manager.getCachePath() / (engineUuidBefore.toString()))
          .replace_extension("asset"));

  manager.loadSourceIfChanged(sourcePath);

  auto engineUuidAfter = manager.findRootAssetUuid(sourcePath);
  EXPECT_TRUE(engineUuidAfter.isValid());
  EXPECT_EQ(engineUuidBefore, engineUuidAfter);
}

TEST_F(
    AssetManagerTest,
    ValidateAndPreloadDoesNotCreateFileWithNewUUIDIfFileContentsHaveChanged) {
  quoll::rhi::MockRenderDevice device;
  quoll::RenderStorage renderStorage(&device);

  fs::create_directories(InnerPathInAssets);

  auto animatorPath = InnerPathInAssets / "test.animator";
  auto metaPath = animatorPath;
  metaPath.replace_extension("animator.meta");

  auto sourcePath = manager.createAnimator(animatorPath).getData();
  auto engineUuidBefore = manager.findRootAssetUuid(sourcePath);
  EXPECT_TRUE(engineUuidBefore.isValid());

  std::filesystem::remove((manager.getCachePath() / engineUuidBefore.toString())
                              .replace_extension("asset"));

  manager.validateAndPreloadAssets(renderStorage);

  auto engineUuidAfter = manager.findRootAssetUuid(sourcePath);
  EXPECT_TRUE(engineUuidAfter.isValid());
  EXPECT_EQ(engineUuidBefore, engineUuidAfter);
}

TEST_F(AssetManagerTest,
       ValidateAndPreloadDeletesCacheFileIfAssetFileDoesNotExist) {
  auto texturePath = CachePath / "test.asset";

  quoll::rhi::MockRenderDevice device;
  quoll::RenderStorage renderStorage(&device);

  createEmptyFile(texturePath);

  EXPECT_TRUE(fs::exists(texturePath));

  manager.validateAndPreloadAssets(renderStorage);

  EXPECT_FALSE(fs::exists(texturePath));
}

TEST_P(AssetTest, FailedImportDoesNotCreateAssetInCache) {
  auto extension = std::get<0>(GetParam());
  // Lua scripts are not compiled at load stage
  // They are only compiled on start by the scripting
  // manager
  if (extension == "lua") {
    return;
  }

  auto filename = "empty-asset." + extension;

  std::ofstream stream(TempPath / filename, std::ios::binary);
  stream.close();

  auto res = manager.importAsset(TempPath / filename, AssetsPath);

  EXPECT_TRUE(res.hasError());
  EXPECT_FALSE(fs::exists(AssetsPath / filename));
}

TEST_P(AssetTest, ImportCopiesSourceToAssets) {
  auto extension = std::get<0>(GetParam());
  auto filename = "valid-asset." + extension;

  auto res = manager.importAsset(FixturesPath / filename, AssetsPath);

  EXPECT_TRUE(res.hasData());
  EXPECT_TRUE(fs::exists(AssetsPath / filename));
}

TEST_P(AssetTest, ImportModifiesTheNameAndCopiesSourceToAssetsIfDuplicate) {
  auto fixtureExtension = std::get<0>(GetParam());
  auto fixturePath =
      (FixturesPath / "valid-asset").replace_extension(fixtureExtension);

  {
    auto res = manager.importAsset(fixturePath, AssetsPath);
    EXPECT_TRUE(res.hasData());
    EXPECT_TRUE(fs::exists(res.getData()));
    EXPECT_EQ(res.getData(),
              (AssetsPath / "valid-asset").replace_extension(fixtureExtension));
  }

  for (u32 i = 1; i < 10; ++i) {
    auto duplicateName = (AssetsPath / ("valid-asset-" + std::to_string(i)))
                             .replace_extension(fixtureExtension);

    auto res = manager.importAsset(fixturePath, AssetsPath);
    EXPECT_TRUE(res.hasData());
    EXPECT_EQ(res.getData(), duplicateName);
    EXPECT_TRUE(fs::exists(duplicateName))
        << duplicateName << " does not exist";
  }
}

TEST_P(AssetTest, ImportCreatesAssetInCache) {
  auto extension = std::get<0>(GetParam());
  auto filename = "valid-asset." + extension;

  auto res = manager.importAsset(FixturesPath / filename, AssetsPath);

  auto uuid = manager.findRootAssetUuid(res.getData());

  EXPECT_TRUE(res.hasData());
  EXPECT_TRUE(uuid.isValid());

  EXPECT_EQ(
      manager.getCache().getAssetMeta(uuid).type,
      quoll::editor::AssetManager::getAssetTypeFromExtension(res.getData()));
}

TEST_P(AssetTest, ImportCreatesMetaFileInSourceDirectory) {
  auto fixtureExt = std::get<0>(GetParam());
  auto fixturePath = FixturesPath / ("valid-asset." + fixtureExt);

  auto res = manager.importAsset(fixturePath, InnerPathInAssets);
  EXPECT_TRUE(res.hasData());

  auto sourcePath = res.getData();
  auto metaPath = sourcePath;
  metaPath.replace_extension(sourcePath.extension().string() + ".meta");
  EXPECT_TRUE(fs::exists(metaPath));

  std::ifstream stream(metaPath);
  auto node = YAML::Load(stream);
  auto sourceAssetHash = node["sourceHash"].as<quoll::String>();
  auto uuid = node["uuid"]["root"].as<quoll::String>();
  auto revision = node["revision"].as<u32>();
  stream.close();

  EXPECT_EQ(uuid.size(), 32);
  EXPECT_NE(revision, 0);

  {
    std::ifstream stream(sourcePath, std::ios::binary);

    quoll::String calculatedHash;
    CryptoPP::SHA256 sha256;
    CryptoPP::FileSource source(
        stream, true,
        new CryptoPP::HashFilter(
            sha256, new CryptoPP::HexEncoder(
                        new CryptoPP::StringSink(calculatedHash))));

    EXPECT_EQ(sourceAssetHash, calculatedHash);

    stream.close();
  }
}

InitAssetTestSuite(AssetManagerTexture,
                   quoll::editor::AssetManager::TextureExtensions,
                   [](auto str) {
                     return str == "ktx2" ? str : (str + ".ktx2");
                   });

InitAssetTestSuite(AssetManagerAudio,
                   quoll::editor::AssetManager::AudioExtensions,
                   [](auto str) { return str; });

InitAssetTestSuite(AssetManagerScript,
                   quoll::editor::AssetManager::ScriptExtensions,
                   [](auto str) { return str; });

InitAssetTestSuite(AssetManagerAnimator,
                   quoll::editor::AssetManager::AnimatorExtensions,
                   [](auto str) { return str; });

InitAssetTestSuite(AssetManagerInputMap,
                   quoll::editor::AssetManager::InputMapExtensions,
                   [](auto str) { return str; });

InitAssetTestSuite(AssetManagerFont,
                   quoll::editor::AssetManager::FontExtensions,
                   [](auto str) { return str; });
