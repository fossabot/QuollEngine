#include "quoll/core/Base.h"
#include "quoll-tests/Testing.h"

#include "ScriptingInterfaceTestBase.h"

static const quoll::Path CachePath = std::filesystem::current_path() / "cache";

const quoll::String LuaScriptingInterfaceTestBase::ScriptName =
    "scripting-system-component-tester.lua";

LuaScriptingInterfaceTestBase::LuaScriptingInterfaceTestBase(
    const quoll::String &scriptName)
    : assetCache(CachePath),
      scriptingSystem(eventSystem, assetCache.getRegistry()),
      mScriptName(scriptName), physicsSystem(physicsBackend) {}

sol::state_view
LuaScriptingInterfaceTestBase::call(quoll::Entity entity,
                                    const quoll::String &functionName) {
  auto handle = loadScript(mScriptName);
  entityDatabase.set<quoll::LuaScript>(entity, {handle});

  scriptingSystem.start(entityDatabase, physicsSystem);

  auto &script = entityDatabase.get<quoll::LuaScript>(entity);
  sol::state_view state(script.state);

  state["assert_native"] = [](bool value) { return value; };

  auto fnRes = state[functionName]();
  if (!fnRes.valid()) {
    sol::error error = fnRes;
    QuollAssert(false, error.what());
  }

  return state;
}

quoll::LuaScriptAssetHandle
LuaScriptingInterfaceTestBase::loadScript(quoll::String scriptName) {
  auto uuid = quoll::Uuid::generate();
  assetCache.createLuaScriptFromSource(FixturesPath / scriptName, uuid);

  auto res = assetCache.loadLuaScript(uuid);
  QuollAssert(res.hasData(), "Error loading script");
  return res.getData();
}

void LuaScriptingInterfaceTestBase::SetUp() {
  TearDown();
  std::filesystem::create_directory(CachePath);
}

void LuaScriptingInterfaceTestBase::TearDown() {
  std::filesystem::remove_all(CachePath);
}
