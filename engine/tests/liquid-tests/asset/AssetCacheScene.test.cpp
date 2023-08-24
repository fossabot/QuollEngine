#include "liquid/core/Base.h"

#include "liquid/core/Version.h"
#include "liquid/asset/AssetCache.h"
#include "liquid/yaml/Yaml.h"

#include "liquid-tests/Testing.h"
#include "liquid-tests/test-utils/AssetCacheTestBase.h"

class AssetCacheSceneTest : public AssetCacheTestBase {
public:
};

TEST_F(AssetCacheSceneTest, CreatesSceneFromSource) {
  auto filePath = cache.createSceneFromSource(FixturesPath / "test.scene", "");
  EXPECT_TRUE(filePath.hasData());
  EXPECT_FALSE(filePath.hasError());
  EXPECT_FALSE(filePath.hasWarnings());

  EXPECT_EQ(filePath.getData().filename().string().size(), 38);

  auto meta = cache.getMetaFromUuid(filePath.getData().stem().string());

  EXPECT_EQ(meta.type, liquid::AssetType::Scene);
  EXPECT_EQ(meta.name, "test.scene");
}

TEST_F(AssetCacheSceneTest, LoadSceneFailsIfTypeIsInvalid) {
  YAML::Node node;
  node["version"] = "0.1";
  node["type"] = "test";
  node["name"] = "test scene";
  node["zones"] = YAML::Node(YAML::NodeType::Sequence);
  node["entities"] = YAML::Node(YAML::NodeType::Sequence);

  std::ofstream stream(CachePath / "test.asset");
  stream << node;
  stream.close();

  auto res = cache.loadSceneFromFile(CachePath / "test.asset");
  EXPECT_FALSE(res.hasData());
  EXPECT_TRUE(res.hasError());
  EXPECT_FALSE(res.hasWarnings());
}

TEST_F(AssetCacheSceneTest, LoadSceneFailsIfVersionIsInvalid) {
  YAML::Node node;
  node["version"] = "0.2";
  node["type"] = "scene";
  node["name"] = "test scene";
  node["zones"] = YAML::Node(YAML::NodeType::Sequence);
  node["entities"] = YAML::Node(YAML::NodeType::Sequence);

  std::ofstream stream(CachePath / "test.asset");
  stream << node;
  stream.close();

  auto res = cache.loadSceneFromFile(CachePath / "test.asset");
  EXPECT_FALSE(res.hasData());
  EXPECT_TRUE(res.hasError());
  EXPECT_FALSE(res.hasWarnings());
}

TEST_F(AssetCacheSceneTest, LoadSceneFailsIfZonesFieldIsNotSequence) {
  std::vector<YAML::Node> invalidNodes{
      YAML::Node(YAML::NodeType::Null),
      YAML::Node(YAML::NodeType::Map),
      YAML::Node(YAML::NodeType::Scalar),
  };

  for (auto invNode : invalidNodes) {
    YAML::Node node;
    node["version"] = "0.1";
    node["type"] = "scene";
    node["name"] = "test scene";
    node["entities"] = YAML::Node(YAML::NodeType::Sequence);
    node["zones"] = invNode;

    std::ofstream stream(CachePath / "test.asset");
    stream << node;
    stream.close();

    auto res = cache.loadSceneFromFile(CachePath / "test.asset");
    EXPECT_FALSE(res.hasData());
    EXPECT_TRUE(res.hasError());
    EXPECT_FALSE(res.hasWarnings());
  }
}

TEST_F(AssetCacheSceneTest, LoadSceneFailsIfEntitiesFieldIsNotSequence) {
  std::vector<YAML::Node> invalidNodes{
      YAML::Node(YAML::NodeType::Null),
      YAML::Node(YAML::NodeType::Map),
      YAML::Node(YAML::NodeType::Scalar),
  };

  for (auto invNode : invalidNodes) {
    YAML::Node node;
    node["version"] = "0.1";
    node["type"] = "scene";
    node["name"] = "test scene";
    node["zones"] = YAML::Node(YAML::NodeType::Sequence);
    node["entities"] = invNode;

    std::ofstream stream(CachePath / "test.asset");
    stream << node;
    stream.close();

    auto res = cache.loadSceneFromFile(CachePath / "test.asset");
    EXPECT_FALSE(res.hasData());
    EXPECT_TRUE(res.hasError());
    EXPECT_FALSE(res.hasWarnings());
  }
}