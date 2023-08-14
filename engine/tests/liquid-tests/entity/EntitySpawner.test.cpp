#include "liquid/core/Base.h"
#include "liquid/entity/EntitySpawner.h"

#include "liquid-tests/Testing.h"

class EntitySpawnerTest : public ::testing::Test {
public:
  EntitySpawnerTest() : entitySpawner(entityDatabase, assetRegistry) {}

  liquid::EntityDatabase entityDatabase;
  liquid::AssetRegistry assetRegistry;
  liquid::EntitySpawner entitySpawner;
};

using EntitySpawnerDeathTest = EntitySpawnerTest;

TEST_F(EntitySpawnerTest, SpawnEmptyCreatesEmptyEntity) {
  liquid::LocalTransform transform{glm::vec3(0.5f, 0.5f, 0.5f)};

  auto entity = entitySpawner.spawnEmpty(transform);
  EXPECT_TRUE(entityDatabase.exists(entity));
  EXPECT_TRUE(entityDatabase.has<liquid::LocalTransform>(entity));
  EXPECT_TRUE(entityDatabase.has<liquid::WorldTransform>(entity));
  EXPECT_TRUE(entityDatabase.has<liquid::Name>(entity));

  EXPECT_EQ(entityDatabase.get<liquid::LocalTransform>(entity).localPosition,
            transform.localPosition);
}

TEST_F(EntitySpawnerDeathTest, SpawnPrefabFailsIfPrefabDoesNotExist) {
  EXPECT_DEATH(entitySpawner.spawnPrefab(liquid::PrefabAssetHandle{15}, {}),
               ".*");
}

TEST_F(EntitySpawnerDeathTest, SpawnPrefabReturnsEmptyListIfPrefabIsEmpty) {
  auto prefab = assetRegistry.getPrefabs().addAsset({});
  EXPECT_DEATH(entitySpawner.spawnPrefab(prefab, {}), ".*");
}

TEST_F(EntitySpawnerTest, SpawnPrefabCreatesEntitiesFromPrefab) {
  liquid::AssetData<liquid::PrefabAsset> asset{};

  // Create 5 transforms
  for (int32_t i = 0; i < 5; ++i) {
    glm::vec3 position(static_cast<float>(i));
    glm::quat rotation(static_cast<float>(i) / 5.0f, 0.0f, 0.0f, 1.0f);
    glm::vec3 scale = position;

    liquid::PrefabComponent<liquid::PrefabTransformData> transform{};
    transform.entity = i;
    transform.value.position = position;
    transform.value.rotation = rotation;
    transform.value.scale = scale;
    transform.value.parent = i - 1;
    asset.data.transforms.push_back(transform);
  }

  // Create two meshes
  for (uint32_t i = 0; i < 2; ++i) {
    liquid::PrefabComponent<liquid::MeshAssetHandle> mesh{};
    mesh.entity = i;
    mesh.value = liquid::MeshAssetHandle{i};
    asset.data.meshes.push_back(mesh);
  }

  // Create two mesh renderers with 3 materials each
  for (uint32_t i = 0; i < 2; ++i) {
    liquid::PrefabComponent<liquid::MeshRenderer> renderer{};
    renderer.entity = i;
    renderer.value.materials.push_back(liquid::MaterialAssetHandle{1});
    renderer.value.materials.push_back(liquid::MaterialAssetHandle{2});
    renderer.value.materials.push_back(liquid::MaterialAssetHandle{3});
    asset.data.meshRenderers.push_back(renderer);
  }

  // Create three skinned mesh renderers with 2 materials each
  for (uint32_t i = 0; i < 3; ++i) {
    liquid::PrefabComponent<liquid::SkinnedMeshRenderer> renderer{};
    renderer.entity = i;
    renderer.value.materials.push_back(liquid::MaterialAssetHandle{1});
    renderer.value.materials.push_back(liquid::MaterialAssetHandle{2});
    asset.data.skinnedMeshRenderers.push_back(renderer);
  }

  // Create names
  for (uint32_t i = 3; i < 5; ++i) {
    liquid::PrefabComponent<liquid::String> name{};
    name.entity = i;
    name.value = "Test name " + std::to_string(i);
    asset.data.names.push_back(name);
  }

  // Create three skinned meshes
  for (uint32_t i = 2; i < 5; ++i) {
    liquid::PrefabComponent<liquid::SkinnedMeshAssetHandle> mesh{};
    mesh.entity = i;
    mesh.value = liquid::SkinnedMeshAssetHandle{i};
    asset.data.skinnedMeshes.push_back(mesh);
  }

  // create skeletons for skinned meshes
  for (uint32_t i = 2; i < 5; ++i) {
    liquid::PrefabComponent<liquid::SkeletonAssetHandle> skeleton{};
    skeleton.entity = i;
    skeleton.value = assetRegistry.getSkeletons().addAsset({});
    asset.data.skeletons.push_back(skeleton);
  }

  for (uint32_t i = 2; i < 5; ++i) {
    liquid::AnimationAssetHandle animation{i};
    asset.data.animations.push_back(animation);
  }

  // create animators for skinned meshes
  // also create one additional animator
  for (uint32_t i = 2; i < 5; ++i) {
    liquid::AssetData<liquid::AnimatorAsset> animatorAsset{};
    animatorAsset.data.initialState = i;
    auto handle = assetRegistry.getAnimators().addAsset(animatorAsset);

    liquid::PrefabComponent<liquid::AnimatorAssetHandle> animator{};
    animator.entity = i;
    animator.value = handle;
    asset.data.animators.push_back(animator);
  }

  // Create two directional lights
  for (uint32_t i = 1; i < 3; ++i) {
    liquid::PrefabComponent<liquid::DirectionalLight> light{};
    light.entity = i;
    light.value.intensity = 25.0f;
    asset.data.directionalLights.push_back(light);
  }

  // Create two point lights
  for (uint32_t i = 3; i < 5; ++i) {
    liquid::PrefabComponent<liquid::PointLight> light{};
    light.entity = i;
    light.value.range = 25.0f;
    asset.data.pointLights.push_back(light);
  }

  auto prefab = assetRegistry.getPrefabs().addAsset(asset);

  liquid::LocalTransform transform{glm::vec3(0.5f, 0.5f, 0.5f)};
  auto res = entitySpawner.spawnPrefab(prefab, transform);

  EXPECT_EQ(res.size(), 5);

  auto &db = entityDatabase;

  // Test relations
  // First item has no parent
  EXPECT_FALSE(db.has<liquid::Parent>(res.at(0)));
  EXPECT_EQ(db.get<liquid::Children>(res.at(0)).children.at(0), res.at(1));

  for (uint32_t i = 1; i < 5; ++i) {
    auto current = res.at(i);
    auto parent = res.at(i - 1);
    // All transform items have parent that is the previous item
    EXPECT_EQ(db.get<liquid::Parent>(current).parent, parent);
    EXPECT_EQ(db.get<liquid::Children>(parent).children.at(0), current);
  }

  // Test transforms

  // First item becomes the root node
  // if it is the only root node
  EXPECT_EQ(db.get<liquid::LocalTransform>(res.at(0)).localPosition,
            transform.localPosition);
  EXPECT_TRUE(db.has<liquid::WorldTransform>(res.at(0)));

  for (uint32_t i = 1; i < 5; ++i) {
    auto entity = res.at(i);

    const auto &transform = db.get<liquid::LocalTransform>(entity);
    const auto &pTransform = asset.data.transforms.at(i).value;
    EXPECT_EQ(transform.localPosition, pTransform.position);
    EXPECT_EQ(transform.localRotation, pTransform.rotation);
    EXPECT_EQ(transform.localScale, pTransform.scale);
    EXPECT_TRUE(db.has<liquid::WorldTransform>(entity));
  }

  // Test names

  // First three items have names with Untitled
  for (uint32_t i = 0; i < 3; ++i) {
    auto entity = res.at(i);
    const auto &name = db.get<liquid::Name>(entity);

    EXPECT_EQ(name.name, "New entity");
  }

  for (uint32_t i = 3; i < 5; ++i) {
    auto entity = res.at(i);
    const auto &name = db.get<liquid::Name>(entity);

    EXPECT_EQ(name.name, "Test name " + std::to_string(i));
  }

  // Test meshes
  for (uint32_t i = 0; i < 2; ++i) {
    auto entity = res.at(i);

    const auto &mesh = db.get<liquid::Mesh>(entity);
    EXPECT_EQ(mesh.handle, static_cast<liquid::MeshAssetHandle>(i));
  }

  // Test mesh renderers
  for (uint32_t i = 0; i < 2; ++i) {
    auto entity = res.at(i);
    const auto &renderer = db.get<liquid::MeshRenderer>(entity);

    for (size_t mi = 0; mi < renderer.materials.size(); ++mi) {
      EXPECT_EQ(renderer.materials.at(mi),
                static_cast<liquid::MaterialAssetHandle>(mi + 1));
    }
  }

  // Test skinned meshes
  for (uint32_t i = 2; i < 5; ++i) {
    auto entity = res.at(i);

    const auto &mesh = db.get<liquid::SkinnedMesh>(entity);
    EXPECT_EQ(mesh.handle, static_cast<liquid::SkinnedMeshAssetHandle>(i));
  }

  // Test skinned mesh renderer
  for (uint32_t i = 0; i < 3; ++i) {
    auto entity = res.at(i);
    const auto &renderer = db.get<liquid::SkinnedMeshRenderer>(entity);

    for (size_t mi = 0; mi < renderer.materials.size(); ++mi) {
      EXPECT_EQ(renderer.materials.at(mi),
                static_cast<liquid::MaterialAssetHandle>(mi + 1));
    }
  }

  // Test skeletons
  for (uint32_t i = 2; i < 5; ++i) {
    auto entity = res.at(i);

    const auto &skeleton = db.get<liquid::Skeleton>(entity);

    // Skeletons vector only has three items
    EXPECT_EQ(skeleton.assetHandle, asset.data.skeletons.at(i - 2).value);
  }

  // Test animators
  for (uint32_t i = 2; i < 5; ++i) {
    auto entity = res.at(i);
    const auto &animator = db.get<liquid::Animator>(entity);
    EXPECT_NE(animator.asset, liquid::AnimatorAssetHandle::Null);
    EXPECT_EQ(animator.currentState, i);
    EXPECT_EQ(
        assetRegistry.getAnimators().getAsset(animator.asset).data.initialState,
        animator.currentState);
    EXPECT_EQ(animator.normalizedTime, 0.0f);
  }

  for (uint32_t i = 1; i < 3; ++i) {
    auto entity = res.at(i);
    const auto &light = db.get<liquid::DirectionalLight>(entity);
    EXPECT_EQ(light.intensity, 25.0f);
  }

  for (uint32_t i = 3; i < 5; ++i) {
    auto entity = res.at(i);
    const auto &light = db.get<liquid::PointLight>(entity);
    EXPECT_EQ(light.range, 25.0f);
  }
}

TEST_F(EntitySpawnerTest, SpawnPrefabCreatesParentsBeforeChild) {
  liquid::AssetData<liquid::PrefabAsset> asset{};

  {
    liquid::PrefabComponent<liquid::PrefabTransformData> transform{};
    transform.entity = 0;
    transform.value.parent = 1;
    transform.value.position = glm::vec3(0.2f);
    asset.data.transforms.push_back(transform);
  }
  auto prefab = assetRegistry.getPrefabs().addAsset(asset);

  liquid::LocalTransform transform{glm::vec3(0.5f, 0.5f, 0.5f)};
  auto res = entitySpawner.spawnPrefab(prefab, transform);
  EXPECT_EQ(res.size(), 2);

  auto &db = entityDatabase;

  auto root = res.at(0);
  auto child = res.at(1);

  EXPECT_EQ(db.get<liquid::LocalTransform>(root).localPosition,
            glm::vec3(0.5f));
  EXPECT_EQ(db.get<liquid::LocalTransform>(child).localPosition,
            glm::vec3(0.2f));
}

TEST_F(
    EntitySpawnerTest,
    SpawnPrefabWrapsAllSpawnedEntitiesInAParentIfPrefabHasMoreThanOneRootEntity) {
  liquid::AssetData<liquid::PrefabAsset> asset{};
  asset.uuid = "231231231";
  asset.name = "my-prefab";

  {
    liquid::PrefabComponent<liquid::PrefabTransformData> transform{};
    transform.entity = 0;
    transform.value.parent = -1;
    asset.data.transforms.push_back(transform);

    liquid::PrefabComponent<liquid::MeshAssetHandle> mesh{};
    mesh.entity = 1;
    asset.data.meshes.push_back(mesh);
  }
  auto prefab = assetRegistry.getPrefabs().addAsset(asset);

  liquid::LocalTransform transform{glm::vec3(0.5f, 0.5f, 0.5f)};
  auto res = entitySpawner.spawnPrefab(prefab, transform);
  EXPECT_EQ(res.size(), 3);

  auto &db = entityDatabase;

  auto root = res.at(res.size() - 1);
  EXPECT_EQ(db.get<liquid::LocalTransform>(root).localPosition,
            transform.localPosition);
  EXPECT_TRUE(db.has<liquid::WorldTransform>(root));
  EXPECT_EQ(db.get<liquid::Name>(root).name, "my-prefab");

  EXPECT_FALSE(entityDatabase.has<liquid::Parent>(root));

  auto children = db.get<liquid::Children>(root).children;
  EXPECT_EQ(children.size(), 2);
  for (size_t i = 0; i < children.size(); ++i) {
    auto entity = res.at(i);
    EXPECT_EQ(children.at(i), entity);
    EXPECT_EQ(db.get<liquid::Parent>(entity).parent, root);
  }
}

TEST_F(EntitySpawnerTest,
       SpawnPrefabCreatesSingleEntityIfPrefabHasOneRootEntity) {
  liquid::AssetData<liquid::PrefabAsset> asset{};
  for (int32_t i = 0; i < 2; ++i) {
    glm::vec3 position(static_cast<float>(i));
    glm::quat rotation(static_cast<float>(i) / 5.0f, 0.0f, 0.0f, 1.0f);
    glm::vec3 scale = position;

    liquid::PrefabComponent<liquid::PrefabTransformData> transform{};
    transform.entity = i;
    transform.value.position = position;
    transform.value.rotation = rotation;
    transform.value.scale = scale;
    transform.value.parent = i - 1;
    asset.data.transforms.push_back(transform);
  }
  auto prefab = assetRegistry.getPrefabs().addAsset(asset);

  liquid::LocalTransform transform{glm::vec3(0.5f, 0.5f, 0.5f)};
  auto res = entitySpawner.spawnPrefab(prefab, transform);
  EXPECT_EQ(res.size(), 2);

  auto &db = entityDatabase;

  auto root = res.at(0);

  EXPECT_EQ(db.get<liquid::LocalTransform>(root).localPosition,
            transform.localPosition);
  EXPECT_TRUE(db.has<liquid::WorldTransform>(root));
  EXPECT_EQ(entityDatabase.get<liquid::Name>(root).name, "New entity");
}

TEST_F(EntitySpawnerTest,
       SpawnSpriteCreatesEntityWithSpriteAndTransformComponents) {
  liquid::AssetData<liquid::TextureAsset> asset{};
  asset.uuid = "121311231";
  asset.name = "my-sprite";
  asset.data.deviceHandle = liquid::rhi::TextureHandle{25};
  auto assetHandle = assetRegistry.getTextures().addAsset(asset);

  liquid::LocalTransform transform{glm::vec3(0.5f, 0.5f, 0.5f)};

  auto entity = entitySpawner.spawnSprite(assetHandle, transform);
  EXPECT_TRUE(entityDatabase.exists(entity));
  EXPECT_EQ(entityDatabase.get<liquid::LocalTransform>(entity).localPosition,
            transform.localPosition);
  EXPECT_TRUE(entityDatabase.has<liquid::WorldTransform>(entity));
  EXPECT_TRUE(entityDatabase.has<liquid::Sprite>(entity));
  EXPECT_EQ(entityDatabase.get<liquid::Sprite>(entity).handle, assetHandle);
  EXPECT_EQ(entityDatabase.get<liquid::Name>(entity).name, "my-sprite");
}
