#include "quoll/core/Base.h"
#include "LuaScriptingSystem.h"
#include "ScriptDecorator.h"

#include "quoll/core/Engine.h"

namespace quoll {

LuaScriptingSystem::LuaScriptingSystem(EventSystem &eventSystem,
                                       AssetRegistry &assetRegistry)
    : mEventSystem(eventSystem), mAssetRegistry(assetRegistry) {}

void LuaScriptingSystem::start(EntityDatabase &entityDatabase,
                               PhysicsSystem &physicsSystem) {
  ScriptGlobals scriptGlobals{entityDatabase, physicsSystem, mAssetRegistry,
                              mScriptLoop};
  QUOLL_PROFILE_EVENT("LuaScriptingSystem::start");
  lua::ScriptDecorator scriptDecorator;
  std::vector<Entity> deleteList;
  std::vector<lua::DeferredLoader *> loaders;
  for (auto [entity, component] : entityDatabase.view<LuaScript>()) {
    if (component.started) {
      continue;
    }

    bool valid = true;
    auto &script = mAssetRegistry.getLuaScripts().getAsset(component.handle);
    for (auto &[key, value] : script.data.variables) {
      auto it = component.variables.find(key);
      if (it == component.variables.end() || !it->second.isType(value.type)) {
        // TODO: Throw error here
        valid = false;
        break;
      }
    }

    if (!valid) {
      deleteList.push_back(entity);
      continue;
    }

    component.loader = [&, entity]() {
      component.started = true;

      if (component.state) {
        mLuaInterpreter.destroyState(component.state);
      }
      component.state = mLuaInterpreter.createState();
      auto state = sol::state_view(component.state);

      scriptDecorator.attachToScope(state, entity, scriptGlobals);
      scriptDecorator.attachVariableInjectors(state, component.variables);

      bool success =
          mLuaInterpreter.evaluate(script.data.bytes, component.state);
      QuollAssert(success, "Cannot evaluate script");
      scriptDecorator.removeVariableInjectors(state);
      createScriptingData(component, entity);
    };

    loaders.push_back(&component.loader);
  }

  for (auto *loader : loaders) {
    loader->wait();
  }

  for (auto entity : deleteList) {
    entityDatabase.remove<LuaScript>(entity);
  }
}

void LuaScriptingSystem::update(f32 dt, EntityDatabase &entityDatabase) {
  QUOLL_PROFILE_EVENT("LuaScriptingSystem::update");

  for (auto [entity, script] : mScriptRemoveObserver) {
    destroyScriptingData(script);
  }
  mScriptRemoveObserver.clear();

  mScriptLoop.getUpdateSignal().notify(dt);
}

void LuaScriptingSystem::cleanup(EntityDatabase &entityDatabase) {
  for (auto [entity, script] : entityDatabase.view<LuaScript>()) {
    destroyScriptingData(script);
  }

  entityDatabase.destroyComponents<LuaScript>();
}

void LuaScriptingSystem::observeChanges(EntityDatabase &entityDatabase) {
  mScriptRemoveObserver = entityDatabase.observeRemove<LuaScript>();
}

void LuaScriptingSystem::createScriptingData(LuaScript &component,
                                             Entity entity) {
  auto state = sol::state_view(component.state);

  if (state["on_collision_start"].get_type() == sol::type::function) {
    component.onCollisionStart = mEventSystem.observe(
        CollisionEvent::CollisionStarted,
        [this, &component, entity](const CollisionObject &data) {
          if (data.a == entity || data.b == entity) {
            auto state = sol::state_view(component.state);
            Entity target = data.a == entity ? data.b : data.a;
            auto table = state.create_table_with("target", target);
            state["on_collision_start"](table);
          }
        });
  }

  if (state["on_collision_end"].get_type() == sol::type::function) {
    component.onCollisionEnd = mEventSystem.observe(
        CollisionEvent::CollisionEnded,
        [this, &component, entity](const CollisionObject &data) {
          auto state = sol::state_view(component.state);

          if (data.a == entity || data.b == entity) {
            Entity target = data.a == entity ? data.b : data.a;
            auto table = state.create_table_with("target", target);
            state["on_collision_end"](table);
          }
        });
  }

  if (state["on_key_press"].get_type() == sol::type::function) {
    component.onKeyPress = mEventSystem.observe(
        KeyboardEvent::Pressed, [this, &component](const auto &data) {
          auto state = sol::state_view(component.state);

          auto table =
              state.create_table_with("key", data.key, "mods", data.mods);
          state["on_key_press"](table);
        });
  }

  if (state["on_key_release"].get_type() == sol::type::function) {
    component.onKeyRelease = mEventSystem.observe(
        KeyboardEvent::Released, [this, &component](const auto &data) {
          auto state = sol::state_view(component.state);

          auto table =
              state.create_table_with("key", data.key, "mods", data.mods);
          state["on_key_release"](table);
        });
  }
}

void LuaScriptingSystem::destroyScriptingData(LuaScript &component) {
  for (auto slot : component.signalSlots) {
    slot.disconnect();
  }

  if (component.onCollisionStart != EventObserverMax) {
    mEventSystem.removeObserver(CollisionEvent::CollisionStarted,
                                component.onCollisionStart);
  }

  if (component.onCollisionEnd != EventObserverMax) {
    mEventSystem.removeObserver(CollisionEvent::CollisionEnded,
                                component.onCollisionEnd);
  }

  if (component.onKeyPress != EventObserverMax) {
    mEventSystem.removeObserver(KeyboardEvent::Pressed, component.onKeyPress);
  }

  if (component.onKeyRelease != EventObserverMax) {
    mEventSystem.removeObserver(KeyboardEvent::Released,
                                component.onKeyRelease);
  }

  if (component.state) {
    mLuaInterpreter.destroyState(component.state);
  }
}

} // namespace quoll
