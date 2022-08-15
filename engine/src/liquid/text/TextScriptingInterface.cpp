#include "liquid/core/Base.h"
#include "TextScriptingInterface.h"

#include "liquid/scripting/LuaScope.h"
#include "liquid/entity/EntityDatabase.h"

namespace liquid {

int TextScriptingInterface::LuaInterface::getText(void *state) {
  LuaScope scope(state);

  if (!scope.is<LuaTable>(1)) {
    // TODO: Print error
    scope.set<String>("");
    return 1;
  }

  auto entityTable = scope.get<LuaTable>(1);
  entityTable.get("id");
  Entity entity = scope.get<uint32_t>();
  scope.pop(2);

  EntityDatabase &entityDatabase = *static_cast<EntityDatabase *>(
      scope.getGlobal<LuaUserData>("__privateDatabase").pointer);

  if (entityDatabase.hasComponent<TextComponent>(entity)) {
    scope.set(entityDatabase.getComponent<TextComponent>(entity).text);
  } else {
    scope.set<String>("");
  }

  return 1;
}

int TextScriptingInterface::LuaInterface::setText(void *state) {
  LuaScope scope(state);
  if (!scope.is<LuaTable>(1) || !scope.is<String>(2)) {
    // TODO: Show logs here
    return 0;
  }

  auto entityTable = scope.get<LuaTable>(1);
  entityTable.get("id");
  Entity entity = scope.get<uint32_t>();
  scope.pop(1);

  auto string = scope.get<String>(2);
  scope.pop(2);

  EntityDatabase &entityDatabase = *static_cast<EntityDatabase *>(
      scope.getGlobal<LuaUserData>("__privateDatabase").pointer);

  // Text needs to exist in order to change it
  if (!entityDatabase.hasComponent<TextComponent>(entity)) {
    return 0;
  }

  entityDatabase.getComponent<TextComponent>(entity).text = string;

  return 0;
};

} // namespace liquid