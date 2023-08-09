#include "liquid/core/Base.h"
#include <random>

#include "liquid/core/Version.h"
#include "liquid/asset/AssetCache.h"
#include "liquid/asset/AssetFileHeader.h"
#include "liquid/schemas/generated/Material.schema.h"
#include "liquid/schemas/FlatbufferHelpers.h"

#include "liquid-tests/Testing.h"
#include "liquid-tests/schemas/generated/Test.schema.h"

class AssetCacheMaterialTest : public ::testing::Test {
public:
  AssetCacheMaterialTest() : cache(FixturesPath) {}

  liquid::AssetData<liquid::MaterialAsset>
  createMaterialAsset(bool createTextures) {
    liquid::AssetData<liquid::MaterialAsset> asset{};
    asset.name = "material1";
    asset.type = liquid::AssetType::Material;
    asset.data.baseColorFactor = glm::vec4(2.5f, 0.2f, 0.5f, 5.2f);
    asset.data.baseColorTextureCoord = 2;

    if (createTextures) {
      liquid::AssetData<liquid::TextureAsset> texture;
      texture.path = liquid::Path(FixturesPath / "textures" / "test.ktx2");
      asset.data.baseColorTexture =
          cache.getRegistry().getTextures().addAsset(texture);
    }

    asset.data.metallicFactor = 1.0f;
    asset.data.roughnessFactor = 2.5f;
    asset.data.metallicRoughnessTextureCoord = 3;
    if (createTextures) {
      liquid::AssetData<liquid::TextureAsset> texture;
      texture.path = liquid::Path(FixturesPath / "textures" / "mr.ktx2");
      asset.data.metallicRoughnessTexture =
          cache.getRegistry().getTextures().addAsset(texture);
    }

    asset.data.normalScale = 0.6f;
    asset.data.normalTextureCoord = 4;
    if (createTextures) {
      liquid::AssetData<liquid::TextureAsset> texture;
      texture.path = liquid::Path(FixturesPath / "textures" / "normal.ktx2");
      asset.data.normalTexture =
          cache.getRegistry().getTextures().addAsset(texture);
    }

    asset.data.occlusionStrength = 0.4f;
    asset.data.occlusionTextureCoord = 5;
    if (createTextures) {
      liquid::AssetData<liquid::TextureAsset> texture;
      texture.path = liquid::Path(FixturesPath / "textures" / "occlusion.ktx2");
      asset.data.occlusionTexture =
          cache.getRegistry().getTextures().addAsset(texture);
    }

    asset.data.emissiveFactor = glm::vec3(0.5f, 0.6f, 2.5f);
    asset.data.emissiveTextureCoord = 6;
    if (createTextures) {
      liquid::AssetData<liquid::TextureAsset> texture;
      texture.path = liquid::Path(FixturesPath / "textures" / "emissive.ktx2");
      asset.data.emissiveTexture =
          cache.getRegistry().getTextures().addAsset(texture);
    }

    return asset;
  }

  liquid::AssetCache cache;
};

TEST_F(AssetCacheMaterialTest, CreatesMaterialWithTexturesAndLoadsItFromFile) {
  auto asset = createMaterialAsset(true);

  auto assetFile = cache.createMaterialFromAsset(asset);
  EXPECT_FALSE(assetFile.hasError());
  EXPECT_FALSE(assetFile.hasWarnings());

  auto res = cache.loadMaterialFromFile(assetFile.getData());

  EXPECT_FALSE(res.hasError());
  EXPECT_FALSE(res.hasWarnings());

  auto handle = res.getData();

  EXPECT_NE(handle, liquid::MaterialAssetHandle::Null);

  auto &material = cache.getRegistry().getMaterials().getAsset(handle);
  EXPECT_EQ(material.name, "material1.material");
  EXPECT_EQ(material.path, FixturesPath / "material1.material");
  EXPECT_EQ(material.type, liquid::AssetType::Material);

  EXPECT_EQ(material.data.baseColorTexture, asset.data.baseColorTexture);
  EXPECT_EQ(material.data.baseColorTextureCoord,
            asset.data.baseColorTextureCoord);
  EXPECT_EQ(material.data.baseColorFactor, asset.data.baseColorFactor);

  EXPECT_EQ(material.data.metallicRoughnessTexture,
            asset.data.metallicRoughnessTexture);
  EXPECT_EQ(material.data.metallicRoughnessTextureCoord,
            asset.data.metallicRoughnessTextureCoord);
  EXPECT_EQ(material.data.metallicFactor, asset.data.metallicFactor);
  EXPECT_EQ(material.data.roughnessFactor, asset.data.roughnessFactor);

  EXPECT_EQ(material.data.normalTexture, asset.data.normalTexture);
  EXPECT_EQ(material.data.normalTextureCoord, asset.data.normalTextureCoord);
  EXPECT_EQ(material.data.normalScale, asset.data.normalScale);

  EXPECT_EQ(material.data.occlusionTexture, asset.data.occlusionTexture);
  EXPECT_EQ(material.data.occlusionTextureCoord,
            asset.data.occlusionTextureCoord);
  EXPECT_EQ(material.data.occlusionStrength, asset.data.occlusionStrength);

  EXPECT_EQ(material.data.emissiveTexture, asset.data.emissiveTexture);
  EXPECT_EQ(material.data.emissiveTextureCoord,
            asset.data.emissiveTextureCoord);
  EXPECT_EQ(material.data.emissiveFactor, asset.data.emissiveFactor);
}

TEST_F(AssetCacheMaterialTest,
       CreatesMaterialWithoutTexturesAndLoadsItFromFile) {
  auto asset = createMaterialAsset(false);

  auto assetFile = cache.createMaterialFromAsset(asset);
  EXPECT_FALSE(assetFile.hasError());
  EXPECT_FALSE(assetFile.hasWarnings());

  auto res = cache.loadMaterialFromFile(assetFile.getData());

  EXPECT_FALSE(res.hasError());
  EXPECT_FALSE(res.hasWarnings());

  EXPECT_EQ(res.getWarnings().size(), 0);

  auto handle = res.getData();

  EXPECT_NE(handle, liquid::MaterialAssetHandle::Null);

  auto &material = cache.getRegistry().getMaterials().getAsset(handle);
  EXPECT_EQ(material.name, "material1.material");
  EXPECT_EQ(material.path, FixturesPath / "material1.material");
  EXPECT_EQ(material.type, liquid::AssetType::Material);

  EXPECT_EQ(material.data.baseColorTexture, asset.data.baseColorTexture);
  EXPECT_EQ(material.data.baseColorTextureCoord,
            asset.data.baseColorTextureCoord);
  EXPECT_EQ(material.data.baseColorFactor, asset.data.baseColorFactor);

  EXPECT_EQ(material.data.metallicRoughnessTexture,
            asset.data.metallicRoughnessTexture);
  EXPECT_EQ(material.data.metallicRoughnessTextureCoord,
            asset.data.metallicRoughnessTextureCoord);
  EXPECT_EQ(material.data.metallicFactor, asset.data.metallicFactor);
  EXPECT_EQ(material.data.roughnessFactor, asset.data.roughnessFactor);

  EXPECT_EQ(material.data.normalTexture, asset.data.normalTexture);
  EXPECT_EQ(material.data.normalTextureCoord, asset.data.normalTextureCoord);
  EXPECT_EQ(material.data.normalScale, asset.data.normalScale);

  EXPECT_EQ(material.data.occlusionTexture, asset.data.occlusionTexture);
  EXPECT_EQ(material.data.occlusionTextureCoord,
            asset.data.occlusionTextureCoord);
  EXPECT_EQ(material.data.occlusionStrength, asset.data.occlusionStrength);

  EXPECT_EQ(material.data.emissiveTexture, asset.data.emissiveTexture);
  EXPECT_EQ(material.data.emissiveTextureCoord,
            asset.data.emissiveTextureCoord);
  EXPECT_EQ(material.data.emissiveFactor, asset.data.emissiveFactor);
}

TEST_F(AssetCacheMaterialTest,
       LoadsTexturesWithMaterialsIfTexturesAreNotLoadedAlready) {
  auto texture = cache.loadTextureFromFile(FixturesPath / "1x1-2d.ktx");
  liquid::AssetData<liquid::MaterialAsset> material{};
  material.name = "test-material";
  material.data.baseColorTexture = texture.getData();
  auto path = cache.createMaterialFromAsset(material);

  cache.getRegistry().getTextures().deleteAsset(texture.getData());
  EXPECT_FALSE(cache.getRegistry().getTextures().hasAsset(texture.getData()));

  auto handle = cache.loadMaterialFromFile(path.getData());

  auto &newMaterial =
      cache.getRegistry().getMaterials().getAsset(handle.getData());

  EXPECT_NE(newMaterial.data.baseColorTexture,
            liquid::TextureAssetHandle::Null);

  auto &newTexture = cache.getRegistry().getTextures().getAsset(
      newMaterial.data.baseColorTexture);
  EXPECT_EQ(newTexture.name, "1x1-2d.ktx");
}

TEST_F(AssetCacheMaterialTest, FailsToLoadMaterialIfFileIdentifierMismatch) {
  flatbuffers::FlatBufferBuilder builder;

  auto emptyStr = builder.CreateString("");

  auto emptyVec4 = liquid::schemas::toFb(glm::vec4{});
  auto emptyVec3 = liquid::schemas::toFb(glm::vec3{});

  liquid::schemas::asset::PBRMetallicRoughnessBuilder pbrBuilder(builder);
  pbrBuilder.add_base_color_texture(emptyStr);
  pbrBuilder.add_base_color_factor(&emptyVec4);
  pbrBuilder.add_metallic_roughness_texture(emptyStr);
  pbrBuilder.add_normal_texture(emptyStr);
  pbrBuilder.add_occlusion_texture(emptyStr);
  pbrBuilder.add_emissive_texture(emptyStr);
  pbrBuilder.add_emissive_factor(&emptyVec3);

  auto pbrMr = pbrBuilder.Finish();

  auto testSchema = liquid::schemas::asset::CreateMaterial(
      builder,
      liquid::schemas::asset::MaterialData::MaterialData_PBRMetallicRoughness,
      pbrMr.Union());
  builder.Finish(testSchema, "TEST");

  const auto *ptr = builder.GetBufferPointer();
  std::ofstream stream(FixturesPath / "invalid.material", std::ios::binary);
  stream.write(reinterpret_cast<const char *>(ptr), builder.GetSize());
  stream.close();

  auto res = cache.loadMaterialFromFile(FixturesPath / "invalid.material");
  EXPECT_TRUE(res.hasError());
}

TEST_F(AssetCacheMaterialTest, FailsToLoadMaterialIfInvalidSchema) {
  flatbuffers::FlatBufferBuilder builder;
  auto testSchema = liquid::schemas::test::CreateTest(
      builder, builder.CreateString("Test string"), 1);
  builder.Finish(testSchema, "LMAT");

  const auto *ptr = builder.GetBufferPointer();
  std::ofstream stream(FixturesPath / "invalid.material", std::ios::binary);
  stream.write(reinterpret_cast<const char *>(ptr), builder.GetSize());
  stream.close();

  auto res = cache.loadMaterialFromFile(FixturesPath / "invalid.material");
  EXPECT_TRUE(res.hasError());
}
