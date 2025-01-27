#include "quoll/core/Base.h"

#include "quoll/core/Name.h"
#include "quoll/core/Id.h"
#include "quoll/core/Delete.h"
#include "quoll/scene/Mesh.h"
#include "quoll/scene/SkinnedMesh.h"
#include "quoll/scene/DirectionalLight.h"
#include "quoll/scene/PointLight.h"
#include "quoll/scene/CascadedShadowMap.h"
#include "quoll/scene/PerspectiveLens.h"
#include "quoll/scene/AutoAspectRatio.h"
#include "quoll/scene/Skeleton.h"
#include "quoll/scene/JointAttachment.h"
#include "quoll/scene/Camera.h"
#include "quoll/scene/Parent.h"
#include "quoll/scene/Children.h"
#include "quoll/scene/LocalTransform.h"
#include "quoll/scene/WorldTransform.h"
#include "quoll/scene/EnvironmentSkybox.h"
#include "quoll/scene/EnvironmentLighting.h"
#include "quoll/scene/Sprite.h"
#include "quoll/animation/Animator.h"
#include "quoll/animation/AnimatorEvent.h"
#include "quoll/audio/AudioSource.h"
#include "quoll/audio/AudioStart.h"
#include "quoll/audio/AudioStatus.h"
#include "quoll/physics/RigidBody.h"
#include "quoll/physics/Collidable.h"
#include "quoll/physics/Force.h"
#include "quoll/physics/Torque.h"
#include "quoll/physics/RigidBodyClear.h"
#include "quoll/text/Text.h"
#include "quoll/renderer/MeshRenderer.h"
#include "quoll/renderer/SkinnedMeshRenderer.h"
#include "quoll/input/InputMap.h"
#include "quoll/physx/PhysxInstance.h"
#include "quoll/lua-scripting/LuaScript.h"
#include "quoll/ui/UICanvas.h"
#include "quoll/ui/UICanvasRenderRequest.h"

#include "SceneLoader.h"

namespace quoll::detail {

SceneLoader::SceneLoader(AssetRegistry &assetRegistry,
                         EntityDatabase &entityDatabase)
    : mAssetRegistry(assetRegistry), mEntityDatabase(entityDatabase) {}

Result<bool> SceneLoader::loadComponents(const YAML::Node &node, Entity entity,
                                         EntityIdCache &entityIdCache) {
  if (node["name"] && node["name"].IsScalar()) {
    auto name = node["name"].as<quoll::String>();
    mEntityDatabase.set<Name>(entity, {name});
  } else {
    auto name = "Untitled " + node["id"].as<String>();
    mEntityDatabase.set<Name>(entity, {name});
  }

  LocalTransform transform{};
  if (node["transform"] && node["transform"].IsMap()) {
    transform.localPosition =
        node["transform"]["position"].as<glm::vec3>(transform.localPosition);

    transform.localRotation =
        node["transform"]["rotation"].as<glm::quat>(transform.localRotation);

    transform.localScale =
        node["transform"]["scale"].as<glm::vec3>(transform.localScale);

    if (node["transform"]["parent"]) {
      auto parentId = node["transform"]["parent"].as<u64>(0);

      auto it = entityIdCache.find(parentId);
      Entity parentEntity =
          it != entityIdCache.end() ? it->second : Entity::Null;

      if (parentEntity != Entity::Null) {
        mEntityDatabase.set<Parent>(entity, {parentEntity});

        if (mEntityDatabase.has<Children>(parentEntity)) {
          mEntityDatabase.get<Children>(parentEntity)
              .children.push_back(entity);
        } else {
          mEntityDatabase.set<Children>(parentEntity, {{entity}});
        }
      }
    }
  }

  mEntityDatabase.set(entity, transform);
  mEntityDatabase.set<WorldTransform>(entity, {});

  if (node["sprite"]) {
    auto uuid = node["sprite"].as<Uuid>(Uuid{});

    auto handle = mAssetRegistry.getTextures().findHandleByUuid(uuid);

    if (handle != TextureAssetHandle::Null) {
      mEntityDatabase.set<Sprite>(entity, {handle});
    }
  }

  if (node["rigidBody"] && node["rigidBody"].IsMap()) {
    RigidBody rigidBody{};
    rigidBody.dynamicDesc.mass =
        node["rigidBody"]["mass"].as<f32>(rigidBody.dynamicDesc.mass);
    rigidBody.dynamicDesc.inertia = node["rigidBody"]["inertia"].as<glm::vec3>(
        rigidBody.dynamicDesc.inertia);
    rigidBody.dynamicDesc.applyGravity =
        node["rigidBody"]["applyGravity"].as<bool>(
            rigidBody.dynamicDesc.applyGravity);

    mEntityDatabase.set(entity, rigidBody);
  }

  static const std::unordered_map<String, PhysicsGeometryType> ValidShapes{
      {"box", PhysicsGeometryType::Box},
      {"sphere", PhysicsGeometryType::Sphere},
      {"capsule", PhysicsGeometryType::Capsule},
      {"plane", PhysicsGeometryType::Plane}};

  if (node["collidable"] && node["collidable"].IsMap() &&
      ValidShapes.find(node["collidable"]["shape"].as<String>("unknown")) !=
          ValidShapes.end()) {
    Collidable collidable{};
    auto shape = ValidShapes.at(node["collidable"]["shape"].as<String>());
    collidable.geometryDesc.type = shape;
    collidable.geometryDesc.center = node["collidable"]["center"].as<glm::vec3>(
        collidable.geometryDesc.center);
    collidable.useInSimulation = node["collidable"]["useInSimulation"].as<bool>(
        collidable.useInSimulation);
    collidable.useInQueries =
        node["collidable"]["useInQueries"].as<bool>(collidable.useInQueries);

    if (shape == PhysicsGeometryType::Box) {
      quoll::PhysicsGeometryBox box{};
      box.halfExtents =
          node["collidable"]["halfExtents"].as<glm::vec3>(box.halfExtents);

      collidable.geometryDesc.params = box;
    } else if (shape == PhysicsGeometryType::Sphere) {
      quoll::PhysicsGeometrySphere sphere{};
      sphere.radius = node["collidable"]["radius"].as<f32>(sphere.radius);

      collidable.geometryDesc.params = sphere;
    } else if (shape == PhysicsGeometryType::Capsule) {
      quoll::PhysicsGeometryCapsule capsule{};
      capsule.radius = node["collidable"]["radius"].as<f32>(capsule.radius);
      capsule.halfHeight =
          node["collidable"]["halfHeight"].as<f32>(capsule.halfHeight);

      collidable.geometryDesc.params = capsule;
    } else if (shape == PhysicsGeometryType::Plane) {
      collidable.geometryDesc.params = PhysicsGeometryPlane{};
    }

    collidable.materialDesc.dynamicFriction =
        node["collidable"]["dynamicFriction"].as<f32>(
            collidable.materialDesc.dynamicFriction);
    collidable.materialDesc.restitution =
        node["collidable"]["restitution"].as<f32>(
            collidable.materialDesc.restitution);
    collidable.materialDesc.staticFriction =
        node["collidable"]["staticFriction"].as<f32>(
            collidable.materialDesc.staticFriction);

    mEntityDatabase.set(entity, collidable);
  }

  if (node["mesh"]) {
    auto uuid = node["mesh"].as<Uuid>(Uuid{});
    auto handle = mAssetRegistry.getMeshes().findHandleByUuid(uuid);

    if (handle != MeshAssetHandle::Null) {
      auto type = mAssetRegistry.getMeshes().getAsset(handle).type;

      if (type == AssetType::Mesh) {
        mEntityDatabase.set<Mesh>(entity, {handle});
      } else if (type == AssetType::SkinnedMesh) {
        mEntityDatabase.set<SkinnedMesh>(entity, {handle});
      }
    }
  }

  if (node["meshRenderer"] && node["meshRenderer"].IsMap()) {
    MeshRenderer renderer{};
    auto materials = node["meshRenderer"]["materials"];

    if (materials.IsSequence()) {
      for (auto material : materials) {
        auto uuid = material.as<Uuid>(Uuid{});

        auto handle = mAssetRegistry.getMaterials().findHandleByUuid(uuid);
        if (handle == MaterialAssetHandle::Null) {
          continue;
        }

        renderer.materials.push_back(handle);
      }
    }

    mEntityDatabase.set(entity, renderer);
  }

  if (node["skinnedMeshRenderer"] && node["skinnedMeshRenderer"].IsMap()) {
    SkinnedMeshRenderer renderer{};
    auto materials = node["skinnedMeshRenderer"]["materials"];

    if (materials.IsSequence()) {
      for (auto material : materials) {
        auto uuid = material.as<Uuid>(Uuid{});
        auto handle = mAssetRegistry.getMaterials().findHandleByUuid(uuid);
        if (handle == MaterialAssetHandle::Null) {
          continue;
        }

        renderer.materials.push_back(handle);
      }
    }

    mEntityDatabase.set(entity, renderer);
  }

  if (node["skeleton"]) {
    auto uuid = node["skeleton"].as<Uuid>(Uuid{});
    auto handle = mAssetRegistry.getSkeletons().findHandleByUuid(uuid);

    if (handle != SkeletonAssetHandle::Null) {
      const auto &skeleton =
          mAssetRegistry.getSkeletons().getAsset(handle).data;

      quoll::Skeleton skeletonComponent{};
      skeletonComponent.jointLocalPositions = skeleton.jointLocalPositions;
      skeletonComponent.jointLocalRotations = skeleton.jointLocalRotations;
      skeletonComponent.jointLocalScales = skeleton.jointLocalScales;
      skeletonComponent.jointParents = skeleton.jointParents;
      skeletonComponent.jointInverseBindMatrices =
          skeleton.jointInverseBindMatrices;
      skeletonComponent.jointNames = skeleton.jointNames;
      skeletonComponent.assetHandle = handle;
      skeletonComponent.numJoints =
          static_cast<u32>(skeleton.jointLocalPositions.size());
      skeletonComponent.jointFinalTransforms.resize(skeletonComponent.numJoints,
                                                    glm::mat4{1.0f});
      skeletonComponent.jointWorldTransforms.resize(skeletonComponent.numJoints,
                                                    glm::mat4{1.0f});

      mEntityDatabase.set(entity, skeletonComponent);
    }
  }

  if (node["jointAttachment"] && node["jointAttachment"].IsMap()) {
    auto joint = node["jointAttachment"]["joint"].as<i16>(-1);
    if (joint >= 0 && joint < std::numeric_limits<u8>::max()) {
      JointAttachment attachment{joint};
      mEntityDatabase.set(entity, attachment);
    }
  }

  if (node["animator"] && node["animator"].IsMap() &&
      node["animator"]["asset"]) {
    auto assetUuid = node["animator"]["asset"].as<Uuid>(Uuid{});
    auto handle = mAssetRegistry.getAnimators().findHandleByUuid(assetUuid);

    if (handle != AnimatorAssetHandle::Null) {
      const auto &asset = mAssetRegistry.getAnimators().getAsset(handle);
      Animator animator;
      animator.asset = handle;
      mEntityDatabase.set(entity, animator);
    }
  }

  if (node["light"] && node["light"].IsMap()) {
    auto light = node["light"];
    auto type = light["type"].as<u32>(std::numeric_limits<u32>::max());

    if (type == 0) {
      DirectionalLight component{};
      component.intensity = light["intensity"].as<f32>(component.intensity);
      component.color = light["color"].as<glm::vec4>(component.color);

      mEntityDatabase.set(entity, component);

      if (light["shadow"] && light["shadow"].IsMap()) {
        CascadedShadowMap shadowComponent{};
        shadowComponent.softShadows = light["shadow"]["softShadows"].as<bool>(
            shadowComponent.softShadows);
        shadowComponent.splitLambda =
            light["shadow"]["splitLambda"].as<f32>(shadowComponent.splitLambda);
        shadowComponent.numCascades =
            light["shadow"]["numCascades"].as<u32>(shadowComponent.numCascades);

        shadowComponent.numCascades = glm::clamp(
            shadowComponent.numCascades, 1u, shadowComponent.MaxCascades);

        shadowComponent.splitLambda =
            glm::clamp(shadowComponent.splitLambda, 0.0f, 1.0f);

        mEntityDatabase.set(entity, shadowComponent);
      }
    } else if (type == 1) {
      PointLight component{};
      component.intensity = light["intensity"].as<f32>(component.intensity);
      component.color = light["color"].as<glm::vec4>(component.color);
      component.range = light["range"].as<glm::float32>(component.range);

      mEntityDatabase.set(entity, component);
    }
  }

  if (node["camera"] && node["camera"].IsMap()) {
    PerspectiveLens lens{};
    f32 near = node["camera"]["near"].as<f32>(lens.near);
    if (near >= 0.0f) {
      lens.near = near;
    }

    f32 far = node["camera"]["far"].as<f32>(lens.far);
    if (far >= 0.0f) {
      lens.far = far;
    }

    glm::vec2 sensorSize =
        node["camera"]["sensorSize"].as<glm::vec2>(lens.sensorSize);

    if (sensorSize.x >= 0.0f && sensorSize.y >= 0.0f) {
      lens.sensorSize = sensorSize;
    }

    f32 focalLength = node["camera"]["focalLength"].as<f32>(lens.focalLength);
    if (focalLength >= 0.0f) {
      lens.focalLength = focalLength;
    }

    f32 aperture = node["camera"]["aperture"].as<f32>(lens.aperture);
    if (aperture >= 0.0f) {
      lens.aperture = aperture;
    }

    f32 shutterSpeed =
        node["camera"]["shutterSpeed"].as<f32>(lens.shutterSpeed);
    if (shutterSpeed >= 0.0f) {
      lens.shutterSpeed = shutterSpeed;
    }

    lens.sensitivity = node["camera"]["sensitivity"].as<u32>(lens.sensitivity);

    bool autoRatio = true;
    if (node["camera"]["aspectRatio"] &&
        node["camera"]["aspectRatio"].IsScalar()) {
      auto res = node["camera"]["aspectRatio"].as<String>("");
      if (res.empty()) {
        res = "auto";
      }
      autoRatio = res == "auto";
    }

    if (autoRatio) {
      mEntityDatabase.set<AutoAspectRatio>(entity, {});
    } else {
      f32 aspectRatio = node["camera"]["aspectRatio"].as<f32>(lens.aspectRatio);
      if (aspectRatio >= 0.0f) {
        lens.aspectRatio = aspectRatio;
      }
    }

    mEntityDatabase.set<Camera>(entity, {});
    mEntityDatabase.set(entity, lens);
  }

  if (node["audio"] && node["audio"].IsMap()) {
    auto uuid = node["audio"]["source"].as<Uuid>(Uuid{});

    auto handle = mAssetRegistry.getAudios().findHandleByUuid(uuid);

    if (handle != AudioAssetHandle::Null) {
      mEntityDatabase.set<AudioSource>(entity, {handle});
    }
  }

  if (node["script"]) {
    LuaScript script{};
    Uuid uuid;
    if (node["script"].IsScalar()) {
      uuid = node["script"].as<Uuid>(Uuid{});
    } else if (node["script"].IsMap()) {
      uuid = node["script"]["asset"].as<Uuid>(Uuid{});

      if (node["script"]["variables"] && node["script"]["variables"].IsMap()) {
        for (const auto &var : node["script"]["variables"]) {
          if (!var.second.IsMap()) {
            continue;
          }
          auto name = var.first.as<String>("");
          auto type = var.second["type"].as<String>("");
          auto value = var.second["value"].as<String>("");

          if (type == "string") {
            script.variables.insert_or_assign(name, value);
          } else if (type == "prefab") {
            auto handle =
                mAssetRegistry.getPrefabs().findHandleByUuid(Uuid(value));
            if (handle != PrefabAssetHandle::Null) {
              script.variables.insert_or_assign(name, handle);
            }
          } else if (type == "texture") {
            auto handle =
                mAssetRegistry.getTextures().findHandleByUuid(Uuid(value));
            if (handle != TextureAssetHandle::Null) {
              script.variables.insert_or_assign(name, handle);
            }
          }
        }
      }
    }

    script.handle = mAssetRegistry.getLuaScripts().findHandleByUuid(uuid);

    if (script.handle != LuaScriptAssetHandle::Null) {
      mEntityDatabase.set(entity, script);
    }
  }

  if (node["text"] && node["text"].IsMap()) {
    auto uuid = node["text"]["font"].as<Uuid>(Uuid{});
    auto handle = mAssetRegistry.getFonts().findHandleByUuid(uuid);

    Text textComponent{};
    textComponent.font = handle;

    if (handle != FontAssetHandle::Null) {
      if (node["text"]["content"] && node["text"]["content"].IsScalar()) {
        textComponent.text =
            node["text"]["content"].as<String>(textComponent.text);
      }

      textComponent.lineHeight =
          node["text"]["lineHeight"].as<f32>(textComponent.lineHeight);

      mEntityDatabase.set(entity, textComponent);
    }
  }

  if (node["skybox"] && node["skybox"].IsMap()) {
    EnvironmentSkybox skybox{};
    auto type = node["skybox"]["type"].as<String>("");
    if (type == "color") {
      skybox.type = EnvironmentSkyboxType::Color;
      skybox.color = node["skybox"]["color"].as<glm::vec4>(
          glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
      mEntityDatabase.set(entity, skybox);
    } else if (type == "texture") {
      auto uuid = node["skybox"]["texture"].as<Uuid>(Uuid{});
      auto handle = mAssetRegistry.getEnvironments().findHandleByUuid(uuid);
      if (handle != EnvironmentAssetHandle::Null) {
        skybox.type = EnvironmentSkyboxType::Texture;
        skybox.texture = handle;
        mEntityDatabase.set(entity, skybox);
      }
    }
  }

  if (node["environmentLighting"] && node["environmentLighting"].IsMap()) {
    auto source = node["environmentLighting"]["source"].as<String>("");
    if (source == "skybox") {
      mEntityDatabase.set<EnvironmentLightingSkyboxSource>(entity, {});
    }
  }

  if (node["inputMap"] && node["inputMap"].IsMap()) {
    auto uuid = node["inputMap"]["asset"].as<Uuid>(Uuid{});
    auto defaultScheme = node["inputMap"]["defaultScheme"].as<usize>(0);
    auto handle = mAssetRegistry.getInputMaps().findHandleByUuid(uuid);

    if (handle != InputMapAssetHandle::Null) {
      auto type = mAssetRegistry.getInputMaps().getAsset(handle).type;

      mEntityDatabase.set<InputMapAssetRef>(entity, {handle, defaultScheme});
    }
  }

  if (node["uiCanvas"] && node["uiCanvas"].IsMap()) {
    mEntityDatabase.set<UICanvas>(entity, {});
  }

  return Result<bool>::Ok(true);
}

Result<Entity> SceneLoader::loadStartingCamera(const YAML::Node &node,
                                               EntityIdCache &entityIdCache) {
  Entity entity = Entity::Null;
  if (node && node.IsScalar()) {
    auto entityId = node.as<u64>(0);

    if (entityId > 0 && entityIdCache.find(entityId) != entityIdCache.end()) {
      auto foundEntity = entityIdCache.at(entityId);

      if (mEntityDatabase.has<PerspectiveLens>(foundEntity)) {
        entity = foundEntity;
      }
    }
  }

  if (entity == Entity::Null) {
    return Result<Entity>::Error("Camera entity not found");
  }

  return Result<Entity>::Ok(entity);
}

Result<Entity> SceneLoader::loadEnvironment(const YAML::Node &node,
                                            EntityIdCache &entityIdCache) {
  if (node && node.IsScalar()) {
    auto entityId = node.as<u64>(0);

    if (entityId > 0 && entityIdCache.contains(entityId)) {
      auto entity = entityIdCache.at(entityId);
      return Result<Entity>::Ok(entity);
    }
  }

  return Result<Entity>::Error("Environment entity not found");
}

} // namespace quoll::detail
