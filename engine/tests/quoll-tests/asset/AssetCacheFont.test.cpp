#include "quoll/core/Base.h"
#include "quoll/asset/AssetCache.h"

#include "quoll-tests/Testing.h"
#include "quoll-tests/test-utils/AssetCacheTestBase.h"

class AssetCacheFontTest : public AssetCacheTestBase {
public:
};

TEST_F(AssetCacheFontTest, CreatesFontFromSource) {
  auto uuid = quoll::Uuid::generate();
  auto sourcePath = FixturesPath / "valid-font.ttf";
  auto filePath = cache.createFontFromSource(sourcePath, uuid);

  EXPECT_TRUE(filePath.hasData());
  EXPECT_FALSE(filePath.hasError());
  EXPECT_FALSE(filePath.hasWarnings());

  EXPECT_EQ(filePath.getData().filename().string().size(), 38);

  auto meta = cache.getAssetMeta(uuid);

  EXPECT_EQ(meta.type, quoll::AssetType::Font);
  EXPECT_EQ(meta.name, "valid-font.ttf");
}

TEST_F(AssetCacheFontTest, LoadsTTFFontFromFile) {
  auto sourcePath = FixturesPath / "valid-font.ttf";
  auto uuid = quoll::Uuid::generate();
  auto filePath = cache.createFontFromSource(sourcePath, uuid);

  auto result = cache.loadFont(uuid);

  EXPECT_TRUE(result.hasData());
  EXPECT_FALSE(result.hasError());
  EXPECT_FALSE(result.hasWarnings());

  auto handle = result.getData();
  EXPECT_NE(handle, quoll::FontAssetHandle::Null);
  const auto &asset = cache.getRegistry().getFonts().getAsset(handle);

  EXPECT_EQ(asset.path, filePath.getData());
  EXPECT_EQ(asset.name, "valid-font.ttf");
  EXPECT_EQ(asset.type, quoll::AssetType::Font);
}

TEST_F(AssetCacheFontTest, LoadsOTFFontFromFile) {
  auto sourcePath = FixturesPath / "valid-font.otf";
  auto uuid = quoll::Uuid::generate();
  auto filePath = cache.createFontFromSource(sourcePath, uuid);

  auto result = cache.loadFont(uuid);

  EXPECT_TRUE(result.hasData());
  EXPECT_FALSE(result.hasError());
  EXPECT_FALSE(result.hasWarnings());

  auto handle = result.getData();
  EXPECT_NE(handle, quoll::FontAssetHandle::Null);
  const auto &asset = cache.getRegistry().getFonts().getAsset(handle);

  EXPECT_EQ(asset.path, filePath.getData());
  EXPECT_EQ(asset.type, quoll::AssetType::Font);
  EXPECT_EQ(asset.name, "valid-font.otf");
}

TEST_F(AssetCacheFontTest, FileReturnsErrorIfFontFileCannotBeOpened) {
  auto result = cache.loadFont(quoll::Uuid::generate());
  EXPECT_TRUE(result.hasError());
  EXPECT_FALSE(result.hasWarnings());
  EXPECT_FALSE(result.hasData());
}
