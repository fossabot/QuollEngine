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

#include "EntitySerializer.h"

namespace quoll::detail {

EntitySerializer::EntitySerializer(AssetRegistry &assetRegistry,
                                   EntityDatabase &entityDatabase)
    : mAssetRegistry(assetRegistry), mEntityDatabase(entityDatabase) {}

Result<YAML::Node> EntitySerializer::serialize(Entity entity) {
  if (!mEntityDatabase.has<Id>(entity)) {
    return Result<YAML::Node>::Error("Entity does not have an ID");
  }

  auto node = createComponentsNode(entity);

  return Result<YAML::Node>::Ok(node);
}

YAML::Node EntitySerializer::createComponentsNode(Entity entity) {
  YAML::Node components;

  if (mEntityDatabase.has<Id>(entity)) {
    components["id"] = mEntityDatabase.get<Id>(entity).id;
  }

  // Set name component if name is empty
  if (!mEntityDatabase.has<Name>(entity) ||
      mEntityDatabase.get<Name>(entity).name.empty()) {

    if (mEntityDatabase.has<Id>(entity)) {
      auto id = mEntityDatabase.get<Id>(entity).id;
      mEntityDatabase.set<Name>(entity, {"Untitled " + std::to_string(id)});
    } else {
      mEntityDatabase.set<Name>(entity, {"Untitled"});
    }
  }
  components["name"] = mEntityDatabase.get<Name>(entity).name;

  if (!mEntityDatabase.has<LocalTransform>(entity)) {
    mEntityDatabase.set<LocalTransform>(entity, {});
  }
  const auto &component = mEntityDatabase.get<LocalTransform>(entity);

  components["transform"]["position"] = component.localPosition;
  components["transform"]["rotation"] = component.localRotation;
  components["transform"]["scale"] = component.localScale;

  if (mEntityDatabase.has<Parent>(entity)) {
    auto parent = mEntityDatabase.get<Parent>(entity).parent;

    if (mEntityDatabase.exists(parent) && mEntityDatabase.has<Id>(parent)) {
      components["transform"]["parent"] = mEntityDatabase.get<Id>(parent).id;
    }
  }

  if (mEntityDatabase.has<DirectionalLight>(entity)) {
    const auto &light = mEntityDatabase.get<DirectionalLight>(entity);

    components["light"]["type"] = 0;
    components["light"]["color"] = light.color;
    components["light"]["intensity"] = light.intensity;

    if (mEntityDatabase.has<CascadedShadowMap>(entity)) {
      const auto &shadow = mEntityDatabase.get<CascadedShadowMap>(entity);
      components["light"]["shadow"]["softShadows"] = shadow.softShadows;
      components["light"]["shadow"]["splitLambda"] = shadow.splitLambda;
      components["light"]["shadow"]["numCascades"] = shadow.numCascades;
    }
  } else if (mEntityDatabase.has<PointLight>(entity)) {
    const auto &light = mEntityDatabase.get<PointLight>(entity);

    components["light"]["type"] = 1;
    components["light"]["color"] = light.color;
    components["light"]["intensity"] = light.intensity;
    components["light"]["range"] = light.range;
  }

  if (mEntityDatabase.has<PerspectiveLens>(entity)) {
    const auto &camera = mEntityDatabase.get<PerspectiveLens>(entity);

    components["camera"]["type"] = 0;
    components["camera"]["near"] = camera.near;
    components["camera"]["far"] = camera.far;
    components["camera"]["aperture"] = camera.aperture;
    components["camera"]["sensorSize"] = camera.sensorSize;
    components["camera"]["focalLength"] = camera.focalLength;
    components["camera"]["shutterSpeed"] = camera.shutterSpeed;
    components["camera"]["sensitivity"] = camera.sensitivity;

    if (mEntityDatabase.has<AutoAspectRatio>(entity)) {
      components["camera"]["aspectRatio"] = "auto";
    } else {
      components["camera"]["aspectRatio"] = camera.aspectRatio;
    }
  }

  if (mEntityDatabase.has<Text>(entity)) {
    const auto &text = mEntityDatabase.get<Text>(entity);

    if (!text.text.empty() && mAssetRegistry.getFonts().hasAsset(text.font)) {
      auto font = mAssetRegistry.getFonts().getAsset(text.font).uuid;

      components["text"]["content"] = text.text;
      components["text"]["lineHeight"] = text.lineHeight;
      components["text"]["font"] = font;
    }
  }

  if (mEntityDatabase.has<RigidBody>(entity)) {
    const auto &rigidBodyDesc =
        mEntityDatabase.get<RigidBody>(entity).dynamicDesc;

    components["rigidBody"]["applyGravity"] = rigidBodyDesc.applyGravity;
    components["rigidBody"]["inertia"] = rigidBodyDesc.inertia;
    components["rigidBody"]["mass"] = rigidBodyDesc.mass;
  }

  if (mEntityDatabase.has<Collidable>(entity)) {
    const auto &component = mEntityDatabase.get<Collidable>(entity);

    auto type = component.geometryDesc.type;

    components["collidable"]["shape"] = getPhysicsGeometryTypeString(type);
    components["collidable"]["center"] = component.geometryDesc.center;
    components["collidable"]["useInSimulation"] = component.useInSimulation;
    components["collidable"]["useInQueries"] = component.useInQueries;

    if (type == PhysicsGeometryType::Box) {
      components["collidable"]["halfExtents"] =
          std::get<PhysicsGeometryBox>(component.geometryDesc.params)
              .halfExtents;
    } else if (type == PhysicsGeometryType::Sphere) {
      components["collidable"]["radius"] =
          std::get<PhysicsGeometrySphere>(component.geometryDesc.params).radius;
    } else if (type == PhysicsGeometryType::Capsule) {
      const auto &capsule =
          std::get<PhysicsGeometryCapsule>(component.geometryDesc.params);
      components["collidable"]["radius"] = capsule.radius;
      components["collidable"]["halfHeight"] = capsule.halfHeight;
    }

    components["collidable"]["dynamicFriction"] =
        component.materialDesc.dynamicFriction;
    components["collidable"]["restitution"] =
        component.materialDesc.restitution;
    components["collidable"]["staticFriction"] =
        component.materialDesc.staticFriction;
  }

  if (mEntityDatabase.has<Sprite>(entity)) {
    auto handle = mEntityDatabase.get<Sprite>(entity).handle;
    if (mAssetRegistry.getTextures().hasAsset(handle)) {
      auto uuid = mAssetRegistry.getTextures().getAsset(handle).uuid;

      components["sprite"] = uuid;
    }
  }

  if (mEntityDatabase.has<Mesh>(entity)) {
    auto handle = mEntityDatabase.get<Mesh>(entity).handle;
    if (mAssetRegistry.getMeshes().hasAsset(handle)) {
      auto uuid = mAssetRegistry.getMeshes().getAsset(handle).uuid;

      components["mesh"] = uuid;
    }
  } else if (mEntityDatabase.has<SkinnedMesh>(entity)) {
    auto handle = mEntityDatabase.get<SkinnedMesh>(entity).handle;
    if (mAssetRegistry.getMeshes().hasAsset(handle)) {
      auto uuid = mAssetRegistry.getMeshes().getAsset(handle).uuid;

      components["mesh"] = uuid;
    }
  }

  if (mEntityDatabase.has<MeshRenderer>(entity)) {
    const auto &renderer = mEntityDatabase.get<MeshRenderer>(entity);

    components["meshRenderer"]["materials"] =
        YAML::Node(YAML::NodeType::Sequence);

    for (auto material : renderer.materials) {
      if (mAssetRegistry.getMaterials().hasAsset(material)) {
        auto uuid = mAssetRegistry.getMaterials().getAsset(material).uuid;
        components["meshRenderer"]["materials"].push_back(uuid);
      }
    }
  }

  if (mEntityDatabase.has<SkinnedMeshRenderer>(entity)) {
    const auto &renderer = mEntityDatabase.get<SkinnedMeshRenderer>(entity);

    components["skinnedMeshRenderer"]["materials"] =
        YAML::Node(YAML::NodeType::Sequence);

    for (auto material : renderer.materials) {
      if (mAssetRegistry.getMaterials().hasAsset(material)) {
        auto uuid = mAssetRegistry.getMaterials().getAsset(material).uuid;
        components["skinnedMeshRenderer"]["materials"].push_back(uuid);
      }
    }
  }

  if (mEntityDatabase.has<Skeleton>(entity)) {
    auto handle = mEntityDatabase.get<Skeleton>(entity).assetHandle;
    if (mAssetRegistry.getSkeletons().hasAsset(handle)) {
      auto uuid = mAssetRegistry.getSkeletons().getAsset(handle).uuid;

      components["skeleton"] = uuid;
    }
  }

  if (mEntityDatabase.has<JointAttachment>(entity)) {
    auto joint = mEntityDatabase.get<JointAttachment>(entity).joint;
    if (joint >= 0) {
      components["jointAttachment"]["joint"] = joint;
    }
  }

  if (mEntityDatabase.has<Animator>(entity)) {
    auto handle = mEntityDatabase.get<Animator>(entity).asset;

    if (mAssetRegistry.getAnimators().hasAsset(handle)) {
      auto uuid = mAssetRegistry.getAnimators().getAsset(handle).uuid;

      components["animator"]["asset"] = uuid;
    }
  }

  if (mEntityDatabase.has<LuaScript>(entity)) {
    const auto &script = mEntityDatabase.get<LuaScript>(entity);
    if (mAssetRegistry.getLuaScripts().hasAsset(script.handle)) {
      const auto &asset =
          mAssetRegistry.getLuaScripts().getAsset(script.handle);

      auto uuid = asset.uuid;

      components["script"]["asset"] = uuid;

      for (auto &[name, value] : script.variables) {
        auto it = asset.data.variables.find(name);
        if (it == asset.data.variables.end() ||
            !value.isType(it->second.type)) {
          continue;
        }

        if (value.isType(LuaScriptVariableType::String)) {
          components["script"]["variables"][name]["type"] = "string";
          components["script"]["variables"][name]["value"] =
              value.get<String>();
        } else if (value.isType(LuaScriptVariableType::AssetPrefab)) {
          auto handle = value.get<PrefabAssetHandle>();
          if (mAssetRegistry.getPrefabs().hasAsset(handle)) {
            auto uuid = mAssetRegistry.getPrefabs().getAsset(handle).uuid;

            components["script"]["variables"][name]["type"] = "prefab";
            components["script"]["variables"][name]["value"] = uuid;
          }
        } else if (value.isType(LuaScriptVariableType::AssetTexture)) {
          auto handle = value.get<TextureAssetHandle>();
          if (mAssetRegistry.getTextures().hasAsset(handle)) {
            auto uuid = mAssetRegistry.getTextures().getAsset(handle).uuid;

            components["script"]["variables"][name]["type"] = "texture";
            components["script"]["variables"][name]["value"] = uuid;
          }
        }
      }
    }
  }

  if (mEntityDatabase.has<AudioSource>(entity)) {
    auto handle = mEntityDatabase.get<AudioSource>(entity).source;
    if (mAssetRegistry.getAudios().hasAsset(handle)) {
      auto uuid = mAssetRegistry.getAudios().getAsset(handle).uuid;

      components["audio"]["source"] = uuid;
    }
  }

  if (mEntityDatabase.has<EnvironmentSkybox>(entity)) {
    const auto &component = mEntityDatabase.get<EnvironmentSkybox>(entity);
    if (component.type == EnvironmentSkyboxType::Color) {
      components["skybox"]["type"] = "color";
      components["skybox"]["color"] = component.color;
    } else if (component.type == EnvironmentSkyboxType::Texture) {
      if (mAssetRegistry.getEnvironments().hasAsset(component.texture)) {
        const auto &asset =
            mAssetRegistry.getEnvironments().getAsset(component.texture);
        components["skybox"]["type"] = "texture";
        components["skybox"]["texture"] = asset.uuid;
      }
    }
  }

  if (mEntityDatabase.has<EnvironmentLightingSkyboxSource>(entity)) {
    components["environmentLighting"]["source"] = "skybox";
  }

  if (mEntityDatabase.has<InputMapAssetRef>(entity)) {
    const auto &component = mEntityDatabase.get<InputMapAssetRef>(entity);

    if (mAssetRegistry.getInputMaps().hasAsset(component.handle)) {
      components["inputMap"]["asset"] =
          mAssetRegistry.getInputMaps().getAsset(component.handle).uuid;
      components["inputMap"]["defaultScheme"] = component.defaultScheme;
    }
  }

  if (mEntityDatabase.has<UICanvas>(entity)) {
    components["uiCanvas"] = YAML::Node(YAML::NodeType::Map);
  }

  return components;
}

} // namespace quoll::detail
