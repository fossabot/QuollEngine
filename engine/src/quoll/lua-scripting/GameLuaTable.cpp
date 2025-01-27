#include "quoll/core/Base.h"
#include "GameLuaTable.h"

namespace quoll {

GameLuaTable::GameLuaTable(Entity entity, ScriptGlobals &scriptGlobals)
    : mEntity(entity), mScriptGlobals(scriptGlobals) {}

lua::ScriptSignalView GameLuaTable::onUpdate() {
  auto &script = mScriptGlobals.entityDatabase.get<LuaScript>(mEntity);
  return lua::ScriptSignalView(mScriptGlobals.scriptLoop.getUpdateSignal(),
                               script);
}

void GameLuaTable::create(sol::state_view state) {
  auto usertype = state.new_usertype<GameLuaTable>("Game", sol::no_constructor);
  usertype["on_update"] = sol::property(&GameLuaTable::onUpdate);
}

} // namespace quoll
