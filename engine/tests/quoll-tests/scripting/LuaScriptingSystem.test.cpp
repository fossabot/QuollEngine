#include "quoll/core/Base.h"
#include "quoll/asset/AssetCache.h"
#include "quoll/entity/EntityLuaTable.h"
#include "quoll/lua-scripting/LuaScriptingSystem.h"

#include "quoll-tests/Testing.h"
#include "quoll-tests/test-utils/AssetCacheTestBase.h"
#include "quoll-tests/test-utils/TestPhysicsBackend.h"

class LuaScriptingSystemTest : public AssetCacheTestBase {
public:
  LuaScriptingSystemTest() : scriptingSystem(eventSystem, cache.getRegistry()) {
    scriptingSystem.observeChanges(entityDatabase);
  }

  quoll::LuaScriptAssetHandle loadLuaScript(quoll::String filename) {
    auto uuid = quoll::Uuid::generate();
    cache.createLuaScriptFromSource(FixturesPath / filename, uuid);
    return cache.loadLuaScript(uuid).getData();
  }

  quoll::EntityDatabase entityDatabase;
  quoll::EventSystem eventSystem;
  quoll::LuaScriptingSystem scriptingSystem;
  quoll::PhysicsSystem physicsSystem{new TestPhysicsBackend};
};

using ScriptingSystemDeathTest = LuaScriptingSystemTest;

static constexpr f32 TimeDelta = 0.2f;

TEST_F(LuaScriptingSystemTest, CallsScriptingUpdateFunctionOnUpdate) {
  auto handle = loadLuaScript("scripting-system-tester.lua");

  auto entity = entityDatabase.create();
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);
  EXPECT_EQ(component.state, nullptr);

  scriptingSystem.start(entityDatabase, physicsSystem);
  EXPECT_NE(component.state, nullptr);

  scriptingSystem.update(TimeDelta, entityDatabase);

  sol::state_view state(component.state);
  EXPECT_EQ(state["value"].get<i32>(), 0);
  EXPECT_EQ(state["global_dt"].get<f32>(), TimeDelta);
}

TEST_F(LuaScriptingSystemTest, DeletesScriptDataWhenComponentIsDeleted) {
  auto handle = loadLuaScript("scripting-system-tester.lua");

  static constexpr usize NumEntities = 20;

  std::vector<quoll::Entity> entities(NumEntities, quoll::Entity::Null);
  for (usize i = 0; i < entities.size(); ++i) {
    auto entity = entityDatabase.create();
    entities.at(i) = entity;

    entityDatabase.set<quoll::LuaScript>(entity, {handle});
  }

  scriptingSystem.start(entityDatabase, physicsSystem);

  std::vector<quoll::LuaScript> scripts(entities.size());
  for (usize i = 0; i < entities.size(); ++i) {
    auto entity = entities.at(i);
    scripts.at(i) = entityDatabase.get<quoll::LuaScript>(entity);
    ASSERT_TRUE(eventSystem.hasObserver(quoll::CollisionEvent::CollisionEnded,
                                        scripts.at(i).onCollisionEnd));

    if ((i % 2) == 0) {
      entityDatabase.remove<quoll::LuaScript>(entity);
    }
  }

  scriptingSystem.update(TimeDelta, entityDatabase);
  for (usize i = 0; i < entities.size(); ++i) {
    auto entity = entities.at(i);
    bool deleted = (i % 2) == 0;
    EXPECT_NE(entityDatabase.has<quoll::LuaScript>(entity), deleted);

    if (deleted) {
      EXPECT_FALSE(eventSystem.hasObserver(
          quoll::CollisionEvent::CollisionEnded, scripts.at(i).onCollisionEnd));
    }
  }
}

TEST_F(LuaScriptingSystemTest, DoesNothingIfScriptHasNoUpdater) {
  auto handle = loadLuaScript("scripting-system-tester.lua");

  auto entity = entityDatabase.create();
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);
  EXPECT_EQ(component.state, nullptr);

  scriptingSystem.start(entityDatabase, physicsSystem);
  EXPECT_NE(component.state, nullptr);

  sol::state_view state(component.state);
  state["disconnect_updater"]();

  for (usize i = 0; i < 10; ++i) {
    scriptingSystem.update(TimeDelta, entityDatabase);
  }

  EXPECT_EQ(state["value"].get<i32>(), -1);
}

TEST_F(LuaScriptingSystemTest, CallsScriptingUpdateFunctionOnEveryUpdate) {
  auto handle = loadLuaScript("scripting-system-tester.lua");

  auto entity = entityDatabase.create();
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);
  EXPECT_EQ(component.state, nullptr);

  scriptingSystem.start(entityDatabase, physicsSystem);
  EXPECT_NE(component.state, nullptr);

  for (usize i = 0; i < 10; ++i) {
    scriptingSystem.update(TimeDelta, entityDatabase);
  }

  sol::state_view state(component.state);
  EXPECT_EQ(state["value"].get<i32>(), 9);
}

TEST_F(LuaScriptingSystemTest, LoadsScriptOnStart) {
  auto handle = loadLuaScript("scripting-system-tester.lua");

  auto entity = entityDatabase.create();
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);
  EXPECT_EQ(component.state, nullptr);

  scriptingSystem.start(entityDatabase, physicsSystem);
  EXPECT_NE(component.state, nullptr);

  sol::state_view state(component.state);
  EXPECT_EQ(state["value"].get<i32>(), -1);
}

TEST_F(LuaScriptingSystemTest, LoadsScriptOnlyOnceOnStart) {
  auto handle = loadLuaScript("scripting-system-tester.lua");

  auto entity = entityDatabase.create();
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);
  EXPECT_EQ(component.state, nullptr);

  // Call 10 times
  for (usize i = 0; i < 10; ++i) {
    scriptingSystem.start(entityDatabase, physicsSystem);
  }
  EXPECT_NE(component.state, nullptr);

  auto state = sol::state_view(component.state);
  EXPECT_EQ(state["value"].get<i32>(), -1);
}

TEST_F(LuaScriptingSystemTest, RemovesScriptComponentIfInputVarsAreNotSet) {
  auto handle = loadLuaScript("scripting-system-vars.lua");

  auto entity = entityDatabase.create();
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);

  scriptingSystem.start(entityDatabase, physicsSystem);
  EXPECT_FALSE(entityDatabase.has<quoll::LuaScript>(entity));
}

TEST_F(LuaScriptingSystemTest,
       RemovesScriptComponentIfInputVarTypesAreInvalid) {
  auto handle = loadLuaScript("scripting-system-vars.lua");

  auto entity = entityDatabase.create();
  quoll::LuaScript script{handle};
  script.variables.insert_or_assign("string_value",
                                    quoll::PrefabAssetHandle{15});
  script.variables.insert_or_assign("prefab_value",
                                    quoll::PrefabAssetHandle{15});
  script.variables.insert_or_assign("texture_value",
                                    quoll::TextureAssetHandle{25});
  entityDatabase.set(entity, script);

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);

  scriptingSystem.start(entityDatabase, physicsSystem);
  EXPECT_FALSE(entityDatabase.has<quoll::LuaScript>(entity));
}

TEST_F(LuaScriptingSystemTest, SetsVariablesToInputVarsOnStart) {
  auto handle = loadLuaScript("scripting-system-vars.lua");

  auto entity = entityDatabase.create();
  quoll::LuaScript script{handle};
  script.variables.insert_or_assign("string_value",
                                    quoll::String("Hello world"));
  script.variables.insert_or_assign("prefab_value",
                                    quoll::PrefabAssetHandle{15});
  script.variables.insert_or_assign("texture_value",
                                    quoll::TextureAssetHandle{25});
  entityDatabase.set(entity, script);

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);

  scriptingSystem.start(entityDatabase, physicsSystem);
  ASSERT_TRUE(entityDatabase.has<quoll::LuaScript>(entity));

  auto state = sol::state_view(component.state);

  EXPECT_EQ(state["var_string"].get<quoll::String>(), "Hello world");
  EXPECT_EQ(state["var_prefab"].get<u32>(), 15);
  EXPECT_EQ(state["var_texture"].get<u32>(), 25);
}

TEST_F(LuaScriptingSystemTest, RemovesVariableSetterAfterInputVariablesAreSet) {
  auto handle = loadLuaScript("scripting-system-vars.lua");

  auto entity = entityDatabase.create();
  quoll::LuaScript script{handle};
  script.variables.insert_or_assign("string_value",
                                    quoll::String("Hello world"));
  script.variables.insert_or_assign("prefab_value",
                                    quoll::PrefabAssetHandle{15});
  script.variables.insert_or_assign("texture_value",
                                    quoll::TextureAssetHandle{25});
  entityDatabase.set(entity, script);

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);

  scriptingSystem.start(entityDatabase, physicsSystem);
  auto state = sol::state_view(component.state);

  EXPECT_EQ(state["var_string"].get<quoll::String>(), "Hello world");
  EXPECT_EQ(state["var_prefab"].get<u32>(), 15);
  EXPECT_EQ(state["var_texture"].get<u32>(), 25);

  EXPECT_TRUE(state["global_vars"].get_type() == sol::type::table);
  EXPECT_TRUE(state["entity"].is<quoll::EntityLuaTable>());

  state["update"]();

  EXPECT_TRUE(state["global_vars"].is<sol::nil_t>());
  EXPECT_TRUE(state["entity"].is<quoll::EntityLuaTable>());
}

TEST_F(LuaScriptingSystemTest, RegistersEventsOnStart) {
  auto handle = loadLuaScript("scripting-system-tester.lua");

  auto entity = entityDatabase.create();
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);

  scriptingSystem.start(entityDatabase, physicsSystem);

  EXPECT_LT(component.onCollisionStart, quoll::EventObserverMax);
  EXPECT_LT(component.onCollisionEnd, quoll::EventObserverMax);
  EXPECT_LT(component.onKeyPress, quoll::EventObserverMax);
  EXPECT_LT(component.onKeyRelease, quoll::EventObserverMax);
}

TEST_F(LuaScriptingSystemTest,
       DoesNotCallScriptCollisionEventIfEntityDidNotCollide) {
  auto handle = loadLuaScript("scripting-system-tester.lua");

  auto entity = entityDatabase.create();
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);

  scriptingSystem.start(entityDatabase, physicsSystem);
  auto state = sol::state_view(component.state);

  eventSystem.dispatch(quoll::CollisionEvent::CollisionStarted,
                       {quoll::Entity{5}, quoll::Entity{6}});
  eventSystem.poll();

  EXPECT_EQ(state["event"].get<i32>(), 0);
}

TEST_F(LuaScriptingSystemTest, CallsScriptCollisionStartEventIfEntityCollided) {
  auto handle = loadLuaScript("scripting-system-tester.lua");

  auto entity = entityDatabase.create();
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);

  scriptingSystem.start(entityDatabase, physicsSystem);
  auto state = sol::state_view(component.state);

  eventSystem.dispatch(quoll::CollisionEvent::CollisionStarted,
                       {entity, quoll::Entity{6}});
  eventSystem.poll();

  EXPECT_EQ(state["event"].get<i32>(), 1);
  EXPECT_EQ(state["target"].get<i32>(), 6);
}

TEST_F(LuaScriptingSystemTest, CallsScriptCollisionEndEventIfEntityCollided) {
  auto handle = loadLuaScript("scripting-system-tester.lua");

  auto entity = entityDatabase.create();
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);

  scriptingSystem.start(entityDatabase, physicsSystem);
  auto state = sol::state_view(component.state);

  eventSystem.dispatch(quoll::CollisionEvent::CollisionEnded,
                       {quoll::Entity{5}, entity});
  eventSystem.poll();

  EXPECT_EQ(state["event"].get<i32>(), 2);
  EXPECT_EQ(state["target"].get<i32>(), 5);
}

TEST_F(LuaScriptingSystemTest, CallsScriptKeyPressEventIfKeyIsPressed) {
  auto handle = loadLuaScript("scripting-system-tester.lua");

  auto entity = entityDatabase.create();
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);

  scriptingSystem.start(entityDatabase, physicsSystem);
  auto state = sol::state_view(component.state);

  eventSystem.dispatch(quoll::KeyboardEvent::Pressed, {15, -1, 3});
  eventSystem.poll();

  EXPECT_EQ(state["event"].get<i32>(), 3);
  EXPECT_EQ(state["key_value"].get<i32>(), 15);
  EXPECT_EQ(state["key_mods"].get<i32>(), 3);
}

TEST_F(LuaScriptingSystemTest, CallsScriptKeyReleaseEventIfKeyIsReleased) {
  auto handle = loadLuaScript("scripting-system-tester.lua");

  auto entity = entityDatabase.create();
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  auto &component = entityDatabase.get<quoll::LuaScript>(entity);

  scriptingSystem.start(entityDatabase, physicsSystem);
  auto state = sol::state_view(component.state);

  eventSystem.dispatch(quoll::KeyboardEvent::Released, {35, -1, 3});
  eventSystem.poll();

  EXPECT_EQ(state["event"].get<i32>(), 4);
  EXPECT_EQ(state["key_value"].get<i32>(), 35);
  EXPECT_EQ(state["key_mods"].get<i32>(), 3);
}
