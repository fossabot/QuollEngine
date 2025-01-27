#include "quoll/core/Base.h"
#include <random>

#include "quoll/core/Version.h"
#include "quoll/asset/AssetCache.h"
#include "quoll/asset/AssetFileHeader.h"
#include "quoll/asset/InputBinaryStream.h"

#include "quoll-tests/Testing.h"
#include "quoll-tests/test-utils/AssetCacheTestBase.h"

class AssetCachePrefabTest : public AssetCacheTestBase {
public:
  quoll::AssetData<quoll::PrefabAsset> createPrefabAsset() {
    quoll::AssetData<quoll::PrefabAsset> asset;
    asset.name = "test-prefab0";
    asset.uuid = quoll::Uuid::generate();

    std::random_device device;
    std::mt19937 mt(device());
    std::uniform_real_distribution<f32> dist(-5.0f, 10.0f);
    std::uniform_int_distribution<i32> idist(-1, 2);
    std::uniform_real_distribution<f32> distColor(0.0f, 1.0f);
    std::uniform_real_distribution<f32> distPositive(0.0f, 10.0f);

    u32 numMaterials = 5;
    u32 numTransforms = 5;
    u32 numMeshes = 5;
    u32 numMeshRenderers = 4;
    u32 numSkeletons = 3;
    u32 numSkinnedMeshRenderers = 2;
    u32 numAnimators = 4;
    u32 numAnimations = 3;
    u32 numDirectionalLights = 2;
    u32 numPointLights = 4;
    u32 numNames = 3;

    for (u32 i = 0; i < numTransforms; ++i) {
      quoll::PrefabTransformData transform{};
      transform.position = {dist(mt), dist(mt), dist(mt)};
      transform.rotation = {dist(mt), dist(mt), dist(mt), dist(mt)};
      transform.scale = {dist(mt), dist(mt), dist(mt)};
      transform.parent = idist(mt);

      asset.data.transforms.push_back({i, transform});
    }

    for (u32 i = 0; i < numNames; ++i) {
      quoll::String name = "Test name " + std::to_string(i);

      asset.data.names.push_back({i, name});
    }

    for (u32 i = 0; i < numDirectionalLights; ++i) {
      quoll::DirectionalLight light{};
      light.color = {distColor(mt), distColor(mt), distColor(mt),
                     distColor(mt)};
      light.intensity = distPositive(mt);

      asset.data.directionalLights.push_back({i, light});
    }

    for (u32 i = 0; i < numPointLights; ++i) {
      quoll::PointLight light{};
      light.color = {distColor(mt), distColor(mt), distColor(mt),
                     distColor(mt)};
      light.intensity = distPositive(mt);
      light.range = distPositive(mt);

      asset.data.pointLights.push_back({i, light});
    }

    std::vector<quoll::MaterialAssetHandle> materials(numMaterials);
    for (u32 i = 0; i < numMaterials; ++i) {
      quoll::AssetData<quoll::MaterialAsset> material;
      material.uuid = quoll::Uuid("material-" + std::to_string(i));
      materials.at(i) = cache.getRegistry().getMaterials().addAsset(material);
    }

    for (u32 i = 0; i < numMeshes; ++i) {
      quoll::AssetData<quoll::MeshAsset> mesh;
      mesh.uuid = quoll::Uuid("mesh-" + std::to_string(i));
      auto handle = cache.getRegistry().getMeshes().addAsset(mesh);
      asset.data.meshes.push_back({i, handle});
    }

    for (u32 i = 0; i < numMeshRenderers; ++i) {
      quoll::MeshRenderer renderer{};
      renderer.materials.push_back(materials.at(i % 3));
      renderer.materials.push_back(materials.at(1 + (i % 3)));

      asset.data.meshRenderers.push_back({i, renderer});
    }

    // Add two more entities that point to the same
    // meshes to test that existing meshes are always
    // referenced instead of added again
    for (u32 i = 0; i < 2; ++i) {
      auto handle = asset.data.meshes.at(static_cast<usize>(i)).value;
      asset.data.meshes.push_back({numMeshes + i, handle});
    }

    for (u32 i = 0; i < numSkinnedMeshRenderers; ++i) {
      quoll::SkinnedMeshRenderer renderer{};
      renderer.materials.push_back(materials.at((i % 2)));
      renderer.materials.push_back(materials.at(1 + (i % 2)));
      renderer.materials.push_back(materials.at(2 + (i % 2)));

      asset.data.skinnedMeshRenderers.push_back({i, renderer});
    }

    for (u32 i = 0; i < numSkeletons; ++i) {
      quoll::AssetData<quoll::SkeletonAsset> skeleton;
      skeleton.uuid = quoll::Uuid("skel-" + std::to_string(i));
      auto handle = cache.getRegistry().getSkeletons().addAsset(skeleton);
      asset.data.skeletons.push_back({i, handle});
    }

    // Add two more entities that point to the same
    // skeletons to test that existing skeletons are always
    // referenced instead of added again
    for (u32 i = 0; i < 2; ++i) {
      auto handle = asset.data.skeletons.at(static_cast<usize>(i)).value;
      asset.data.skeletons.push_back({numSkeletons + i, handle});
    }

    for (u32 i = 0; i < numAnimations; ++i) {
      quoll::AssetData<quoll::AnimationAsset> animation;
      animation.uuid = quoll::Uuid("animation-" + std::to_string(i));

      auto handle = cache.getRegistry().getAnimations().addAsset(animation);
      asset.data.animations.push_back(handle);
    }

    for (u32 i = 0; i < numAnimators; ++i) {
      quoll::AssetData<quoll::AnimatorAsset> animator;
      animator.uuid = quoll::Uuid("animator-" + std::to_string(i));
      auto handle = cache.getRegistry().getAnimators().addAsset(animator);
      asset.data.animators.push_back({i, handle});
    }

    // Add two more entities that point to same animations
    // to test that existing animations are always
    // referenced instead of added again
    for (u32 i = 0; i < 2; ++i) {
      auto handle = asset.data.animators.at(i).value;
      asset.data.animators.push_back({numAnimators + i, handle});
    }

    return asset;
  }

  void SetUp() override {
    AssetCacheTestBase::SetUp();

    textureUuid = quoll::Uuid::generate();
    cache.createTextureFromSource(FixturesPath / "1x1-2d.ktx", textureUuid);
  }

  quoll::Uuid textureUuid;
};

TEST_F(AssetCachePrefabTest, CreatesPrefabFile) {
  auto asset = createPrefabAsset();

  auto filePath = cache.createPrefabFromAsset(asset);
  quoll::InputBinaryStream file(filePath.getData());
  EXPECT_TRUE(file.good());

  quoll::AssetFileHeader header;
  file.read(header);
  quoll::String magic(quoll::AssetFileMagicLength, '$');
  EXPECT_EQ(header.magic, header.MagicConstant);
  EXPECT_EQ(header.type, quoll::AssetType::Prefab);

  std::vector<quoll::Uuid> actualMaterials;
  {
    auto &map = cache.getRegistry().getMaterials();
    auto &actual = actualMaterials;
    u32 numAssets = 0;
    file.read(numAssets);
    EXPECT_EQ(numAssets, 4);
    actualMaterials.resize(4);
    file.read(actual);

    for (u32 i = 0; i < numAssets; ++i) {
      auto handle = map.findHandleByUuid(actual.at(i));
      EXPECT_NE(handle, quoll::MaterialAssetHandle::Null);
    }
  }

  std::vector<quoll::Uuid> actualMeshes;
  {
    auto &expected = asset.data.meshes;
    auto &map = cache.getRegistry().getMeshes();
    u32 numAssets = 0;
    file.read(numAssets);
    EXPECT_EQ(numAssets, static_cast<u32>(expected.size() - 2));
    actualMeshes.resize(numAssets);
    file.read(actualMeshes);

    for (u32 i = 0; i < numAssets; ++i) {
      auto expectedString = map.getAsset(expected.at(i).value).uuid;
      EXPECT_EQ(expectedString, actualMeshes.at(i));
    }
  }

  std::vector<quoll::Uuid> actualSkeletons;
  {
    auto &expected = asset.data.skeletons;
    auto &map = cache.getRegistry().getSkeletons();
    u32 numAssets = 0;
    file.read(numAssets);
    EXPECT_EQ(numAssets, 3);
    actualSkeletons.resize(numAssets);
    file.read(actualSkeletons);

    for (u32 i = 0; i < numAssets; ++i) {
      auto expectedString = map.getAsset(expected.at(i).value).uuid;
      EXPECT_EQ(expectedString, actualSkeletons.at(i));
    }
  }

  std::vector<quoll::Uuid> actualAnimations;
  {
    auto &expected = asset.data.animations;
    auto &map = cache.getRegistry().getAnimations();
    auto &actual = actualAnimations;
    u32 numAssets = 0;
    file.read(numAssets);
    EXPECT_EQ(numAssets, 3);
    actualAnimations.resize(numAssets);
    file.read(actual);

    for (u32 i = 0; i < numAssets; ++i) {
      auto expectedString = map.getAsset(expected.at(i)).uuid;
      EXPECT_EQ(expectedString, actual.at(i));
    }
  }

  std::vector<quoll::Uuid> actualAnimators;
  {
    auto &expected = asset.data.animators;
    auto &map = cache.getRegistry().getAnimators();
    auto &actual = actualAnimators;
    u32 numAssets = 0;
    file.read(numAssets);
    EXPECT_EQ(numAssets, 4);
    actualAnimators.resize(numAssets);
    file.read(actual);

    for (u32 i = 0; i < numAssets; ++i) {
      auto expectedString = map.getAsset(expected.at(i).value).uuid;
      EXPECT_EQ(expectedString, actual.at(i));
    }
  }

  {
    u32 numComponents = 0;
    file.read(numComponents);
    EXPECT_EQ(numComponents, 5);
    std::vector<quoll::PrefabTransformData> transforms(numComponents);
    for (u32 i = 0; i < numComponents; ++i) {
      u32 entity = 0;
      file.read(entity);

      glm::vec3 position;
      glm::quat rotation;
      glm::vec3 scale;
      i32 parent = -1;
      file.read(position);
      file.read(rotation);
      file.read(scale);
      file.read(parent);

      EXPECT_EQ(asset.data.transforms.at(i).entity, entity);
      EXPECT_EQ(asset.data.transforms.at(i).value.position, position);
      EXPECT_EQ(asset.data.transforms.at(i).value.rotation, rotation);
      EXPECT_EQ(asset.data.transforms.at(i).value.scale, scale);
      EXPECT_EQ(asset.data.transforms.at(i).value.parent, parent);
    }
  }

  {
    u32 numComponents = 0;
    file.read(numComponents);
    EXPECT_EQ(numComponents, 3);

    for (u32 i = 0; i < numComponents; ++i) {
      u32 entity = 999;
      file.read(entity);
      EXPECT_EQ(entity, i);

      quoll::String name;
      file.read(name);

      EXPECT_EQ(asset.data.names.at(i).value, name);
    }
  }

  {
    u32 numComponents = 0;
    file.read(numComponents);
    EXPECT_EQ(numComponents, 7);
    auto &map = cache.getRegistry().getMeshes();
    for (u32 i = 0; i < numComponents; ++i) {
      u32 entity = 999;
      file.read(entity);
      EXPECT_EQ(entity, i);

      u32 meshIndex = 999;
      file.read(meshIndex);
      EXPECT_EQ(meshIndex, i % 5);

      auto &expected = map.getAsset(asset.data.meshes.at(i).value);

      EXPECT_EQ(expected.uuid, actualMeshes.at(meshIndex));
    }
  }

  {
    u32 numComponents = 999;
    file.read(numComponents);
    EXPECT_EQ(numComponents, 4);
    auto &map = cache.getRegistry().getMaterials();
    for (u32 i = 0; i < numComponents; ++i) {
      u32 entity = 999;
      file.read(entity);
      EXPECT_EQ(entity, i);

      u32 numMaterials = 999;
      file.read(numMaterials);
      EXPECT_EQ(numMaterials, 2);

      std::vector<u32> materialIndices(numMaterials);
      file.read(materialIndices);

      auto &expected = asset.data.meshRenderers.at(i).value;
      EXPECT_EQ(materialIndices.size(), expected.materials.size());

      for (usize mi = 0; mi < materialIndices.size(); ++mi) {
        auto materialIndex = materialIndices.at(mi);
        auto handle = expected.materials.at(mi);

        auto uuid = actualMaterials.at(materialIndex);
        EXPECT_EQ(uuid, map.getAsset(handle).uuid);
      }
    }
  }

  {
    u32 numComponents = 999;
    file.read(numComponents);
    EXPECT_EQ(numComponents, 2);
    auto &map = cache.getRegistry().getMaterials();
    for (u32 i = 0; i < numComponents; ++i) {
      u32 entity = 999;
      file.read(entity);
      EXPECT_EQ(entity, i);

      u32 numMaterials = 999;
      file.read(numMaterials);
      EXPECT_EQ(numMaterials, 3);

      std::vector<u32> materialIndices(numMaterials);
      file.read(materialIndices);

      auto &expected = asset.data.skinnedMeshRenderers.at(i).value;
      EXPECT_EQ(materialIndices.size(), expected.materials.size());

      for (usize mi = 0; mi < materialIndices.size(); ++mi) {
        auto materialIndex = materialIndices.at(mi);
        auto handle = expected.materials.at(mi);

        auto uuid = actualMaterials.at(materialIndex);
        EXPECT_EQ(uuid, map.getAsset(handle).uuid);
      }
    }
  }

  {
    u32 numComponents = 0;
    file.read(numComponents);
    EXPECT_EQ(numComponents, 5);
    auto &map = cache.getRegistry().getSkeletons();
    for (u32 i = 0; i < numComponents; ++i) {
      u32 entity = 999;
      file.read(entity);
      EXPECT_EQ(entity, i);

      u32 skeletonIndex = 999;
      file.read(skeletonIndex);
      EXPECT_EQ(skeletonIndex, i % 3);

      auto &expected = map.getAsset(asset.data.skeletons.at(i).value);

      EXPECT_EQ(expected.uuid, actualSkeletons.at(skeletonIndex));
    }
  }

  {
    u32 numComponents = 0;
    file.read(numComponents);
    EXPECT_EQ(numComponents, 3);
    auto &map = cache.getRegistry().getAnimations();

    for (u32 i = 0; i < numComponents; ++i) {
      u32 animatorIndex = 999;
      file.read(animatorIndex);

      auto &expected = map.getAsset(asset.data.animations.at(i));

      EXPECT_EQ(expected.uuid, actualAnimations.at(animatorIndex));
    }
  }

  {
    u32 numComponents = 0;
    file.read(numComponents);
    EXPECT_EQ(numComponents, 6);
    auto &map = cache.getRegistry().getAnimators();

    for (u32 i = 0; i < numComponents; ++i) {
      u32 entity = 999;
      file.read(entity);
      EXPECT_EQ(entity, i);

      u32 animatorIndex = 999;
      file.read(animatorIndex);

      auto &expected = map.getAsset(asset.data.animators.at(i).value);

      EXPECT_EQ(expected.uuid, actualAnimators.at(animatorIndex));
    }
  }

  {
    u32 numComponents = 0;
    file.read(numComponents);
    EXPECT_EQ(numComponents, 2);

    for (u32 i = 0; i < numComponents; ++i) {
      u32 entity = 999;
      file.read(entity);
      EXPECT_EQ(entity, i);

      glm::vec4 color{999.0f};
      f32 intensity = 0.0f;
      file.read(color);
      file.read(intensity);

      EXPECT_EQ(asset.data.directionalLights.at(i).value.color, color);
      EXPECT_EQ(asset.data.directionalLights.at(i).value.intensity, intensity);
    }
  }

  {
    u32 numComponents = 0;
    file.read(numComponents);
    EXPECT_EQ(numComponents, 4);

    for (u32 i = 0; i < numComponents; ++i) {
      u32 entity = 999;
      file.read(entity);
      EXPECT_EQ(entity, i);

      glm::vec4 color{999.0f};
      f32 intensity = 0.0f;
      f32 range = 99.0f;
      file.read(color);
      file.read(intensity);
      file.read(range);

      EXPECT_EQ(asset.data.pointLights.at(i).value.color, color);
      EXPECT_EQ(asset.data.pointLights.at(i).value.intensity, intensity);
      EXPECT_EQ(asset.data.pointLights.at(i).value.range, range);
    }
  }

  EXPECT_FALSE(std::filesystem::exists(
      filePath.getData().replace_extension("assetmeta")));
}

TEST_F(AssetCachePrefabTest, FailsLoadingPrefabIfPrefabHasNoComponents) {
  quoll::AssetData<quoll::PrefabAsset> asset;
  asset.uuid = quoll::Uuid::generate();

  auto filePath = cache.createPrefabFromAsset(asset);

  auto res = cache.loadPrefab(asset.uuid);
  EXPECT_TRUE(res.hasError());
  EXPECT_FALSE(res.hasData());
  EXPECT_FALSE(res.hasWarnings());
}

TEST_F(AssetCachePrefabTest, LoadsPrefabFile) {
  auto asset = createPrefabAsset();
  auto filePath = cache.createPrefabFromAsset(asset);
  auto handle = cache.loadPrefab(asset.uuid);
  EXPECT_NE(handle.getData(), quoll::PrefabAssetHandle::Null);
  EXPECT_FALSE(handle.hasWarnings());

  auto &prefab = cache.getRegistry().getPrefabs().getAsset(handle.getData());
  EXPECT_EQ(prefab.name, asset.name);

  EXPECT_EQ(asset.data.transforms.size(), prefab.data.transforms.size());
  for (usize i = 0; i < prefab.data.transforms.size(); ++i) {
    auto &expected = asset.data.transforms.at(i);
    auto &actual = prefab.data.transforms.at(i);
    EXPECT_EQ(expected.entity, actual.entity);
    EXPECT_EQ(expected.value.position, actual.value.position);
    EXPECT_EQ(expected.value.rotation, actual.value.rotation);
    EXPECT_EQ(expected.value.scale, actual.value.scale);
    EXPECT_EQ(expected.value.parent, actual.value.parent);
  }

  EXPECT_EQ(asset.data.names.size(), prefab.data.names.size());
  for (usize i = 0; i < prefab.data.names.size(); ++i) {
    auto &expected = asset.data.names.at(i);
    auto &actual = prefab.data.names.at(i);
    EXPECT_EQ(expected.entity, actual.entity);
    EXPECT_EQ(expected.value, actual.value);
  }

  EXPECT_EQ(asset.data.meshes.size(), prefab.data.meshes.size());
  for (usize i = 0; i < prefab.data.meshes.size(); ++i) {
    auto &expected = asset.data.meshes.at(i);
    auto &actual = prefab.data.meshes.at(i);
    EXPECT_EQ(expected.entity, actual.entity);
    EXPECT_EQ(expected.value, actual.value);
  }

  EXPECT_EQ(asset.data.meshRenderers.size(), prefab.data.meshRenderers.size());
  for (usize i = 0; i < prefab.data.meshRenderers.size(); ++i) {
    auto &expected = asset.data.meshRenderers.at(i);
    auto &actual = prefab.data.meshRenderers.at(i);
    EXPECT_EQ(expected.entity, actual.entity);
    EXPECT_EQ(expected.value.materials.size(), actual.value.materials.size());
    for (usize mi = 0; mi < expected.value.materials.size(); ++mi) {
      EXPECT_EQ(expected.value.materials.at(mi), actual.value.materials.at(mi));
    }
  }

  EXPECT_EQ(asset.data.skinnedMeshRenderers.size(),
            prefab.data.skinnedMeshRenderers.size());
  for (usize i = 0; i < prefab.data.skinnedMeshRenderers.size(); ++i) {
    auto &expected = asset.data.skinnedMeshRenderers.at(i);
    auto &actual = prefab.data.skinnedMeshRenderers.at(i);
    EXPECT_EQ(expected.entity, actual.entity);
    EXPECT_EQ(expected.value.materials.size(), actual.value.materials.size());
    for (usize mi = 0; mi < expected.value.materials.size(); ++mi) {
      EXPECT_EQ(expected.value.materials.at(mi), actual.value.materials.at(mi));
    }
  }

  EXPECT_EQ(asset.data.skeletons.size(), prefab.data.skeletons.size());
  for (usize i = 0; i < prefab.data.skeletons.size(); ++i) {
    auto &expected = asset.data.skeletons.at(i);
    auto &actual = prefab.data.skeletons.at(i);
    EXPECT_EQ(expected.entity, actual.entity);
    EXPECT_EQ(expected.value, actual.value);
  }

  EXPECT_EQ(asset.data.animations.size(), prefab.data.animations.size());
  for (usize i = 0; i < prefab.data.animations.size(); ++i) {
    auto &expected = asset.data.animations.at(i);
    auto &actual = prefab.data.animations.at(i);

    EXPECT_EQ(expected, actual);
  }

  EXPECT_EQ(asset.data.animators.size(), prefab.data.animators.size());
  for (usize i = 0; i < prefab.data.animators.size(); ++i) {
    auto &expected = asset.data.animators.at(i);
    auto &actual = prefab.data.animators.at(i);

    EXPECT_EQ(expected.entity, actual.entity);
    EXPECT_EQ(expected.value, actual.value);
  }

  EXPECT_EQ(asset.data.directionalLights.size(),
            prefab.data.directionalLights.size());
  for (usize i = 0; i < prefab.data.directionalLights.size(); ++i) {
    auto &expected = asset.data.directionalLights.at(i);
    auto &actual = prefab.data.directionalLights.at(i);

    EXPECT_EQ(expected.entity, actual.entity);
    EXPECT_EQ(expected.value.color, actual.value.color);
    EXPECT_EQ(expected.value.intensity, actual.value.intensity);
    EXPECT_EQ(expected.value.direction, glm::vec3(0.0f));
    EXPECT_EQ(actual.value.direction, glm::vec3(0.0f));
  }

  EXPECT_EQ(asset.data.pointLights.size(), prefab.data.pointLights.size());
  for (usize i = 0; i < prefab.data.pointLights.size(); ++i) {
    auto &expected = asset.data.pointLights.at(i);
    auto &actual = prefab.data.pointLights.at(i);

    EXPECT_EQ(expected.entity, actual.entity);
    EXPECT_EQ(expected.value.color, actual.value.color);
    EXPECT_EQ(expected.value.intensity, actual.value.intensity);
    EXPECT_EQ(expected.value.range, actual.value.range);
  }
}

TEST_F(AssetCachePrefabTest, LoadsPrefabWithMeshAnimationSkeleton) {
  // Create texture
  auto tempTextureHandle = cache.loadTexture(textureUuid).getData();
  auto tempTextureAsset =
      cache.getRegistry().getTextures().getAsset(tempTextureHandle);
  cache.getRegistry().getTextures().deleteAsset(tempTextureHandle);

  auto texturePath = cache.createTextureFromAsset(tempTextureAsset).getData();
  auto textureHandle = cache.loadTexture(textureUuid);

  // Create mesh
  quoll::AssetData<quoll::MeshAsset> meshData{};
  meshData.type = quoll::AssetType::SkinnedMesh;
  meshData.uuid = quoll::Uuid::generate();

  quoll::BaseGeometryAsset geometry{};
  geometry.positions.push_back({});
  geometry.normals.push_back({});
  geometry.tangents.push_back({});
  geometry.texCoords0.push_back({});
  geometry.texCoords1.push_back({});
  geometry.joints.push_back({});
  geometry.weights.push_back({});

  geometry.indices.push_back(0);
  meshData.data.geometries.push_back(geometry);
  auto meshPath = cache.createMeshFromAsset(meshData).getData();
  auto meshHandle = cache.loadMesh(meshData.uuid);

  // Create skeleton
  quoll::AssetData<quoll::SkeletonAsset> skeletonData{};
  skeletonData.uuid = quoll::Uuid::generate();

  auto skeletonPath = cache.createSkeletonFromAsset(skeletonData).getData();
  auto skeletonHandle = cache.loadSkeleton(skeletonData.uuid);

  // Create animation
  quoll::AssetData<quoll::AnimationAsset> animationData{};
  animationData.data.time = 2.5;
  animationData.uuid = quoll::Uuid::generate();
  auto animationPath = cache.createAnimationFromAsset(animationData).getData();
  auto animationHandle = cache.loadAnimation(animationData.uuid);

  // Create animator
  quoll::AssetData<quoll::AnimatorAsset> animatorData{};
  animatorData.data.states.push_back({"INITIAL"});
  animatorData.uuid = quoll::Uuid::generate();
  auto animatorPath = cache.createAnimatorFromAsset(animatorData).getData();
  auto animatorHandle = cache.loadAnimator(animatorData.uuid);

  // Create prefab
  quoll::AssetData<quoll::PrefabAsset> prefabData{};
  prefabData.uuid = quoll::Uuid::generate();
  prefabData.data.meshes.push_back({0U, meshHandle.getData()});
  prefabData.data.skeletons.push_back({0U, skeletonHandle.getData()});
  prefabData.data.animations.push_back(animationHandle.getData());
  prefabData.data.animators.push_back({0U, animatorHandle.getData()});

  auto prefabPath = cache.createPrefabFromAsset(prefabData);

  // Delete all existing assets
  cache.getRegistry().getTextures().deleteAsset(textureHandle.getData());
  cache.getRegistry().getMeshes().deleteAsset(meshHandle.getData());
  cache.getRegistry().getSkeletons().deleteAsset(skeletonHandle.getData());
  cache.getRegistry().getAnimations().deleteAsset(animationHandle.getData());
  cache.getRegistry().getAnimators().deleteAsset(animatorHandle.getData());

  auto prefabHandle = cache.loadPrefab(prefabData.uuid);
  EXPECT_NE(prefabHandle.getData(), quoll::PrefabAssetHandle::Null);

  auto &newPrefab =
      cache.getRegistry().getPrefabs().getAsset(prefabHandle.getData());

  // Validate mesh
  EXPECT_NE(newPrefab.data.meshes.at(0).value, quoll::MeshAssetHandle::Null);
  auto &newMesh = cache.getRegistry().getMeshes().getAsset(
      newPrefab.data.meshes.at(0).value);
  EXPECT_EQ(newMesh.path, meshPath);

  // Validate skeleton
  EXPECT_NE(newPrefab.data.skeletons.at(0).value,
            quoll::SkeletonAssetHandle::Null);
  auto &newSkeleton = cache.getRegistry().getSkeletons().getAsset(
      newPrefab.data.skeletons.at(0).value);
  EXPECT_EQ(newSkeleton.path, skeletonPath);

  // Validate animation
  auto newAnimationHandle = newPrefab.data.animations.at(0);
  EXPECT_NE(newAnimationHandle, quoll::AnimationAssetHandle::Null);
  auto &newAnimation =
      cache.getRegistry().getAnimations().getAsset(newAnimationHandle);
  EXPECT_EQ(newAnimation.path, animationPath);

  // Validate animator
  auto newAnimatorHandle = newPrefab.data.animators.at(0).value;
  EXPECT_NE(newAnimatorHandle, quoll::AnimatorAssetHandle::Null);
  auto &newAnimator =
      cache.getRegistry().getAnimators().getAsset(newAnimatorHandle);
  EXPECT_EQ(newAnimator.path, animatorPath);
}
