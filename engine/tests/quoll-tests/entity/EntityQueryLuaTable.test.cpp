#include "quoll/core/Base.h"
#include "quoll/core/Name.h"
#include "quoll/core/Delete.h"
#include "quoll/entity/EntityLuaTable.h"

#include "quoll-tests/Testing.h"
#include "quoll-tests/test-utils/ScriptingInterfaceTestBase.h"

using EntityQueryLuaTableTest = LuaScriptingInterfaceTestBase;

TEST_F(EntityQueryLuaTableTest,
       GetEntityByNameReturnsNulllIfEntityDoesNotExist) {
  auto entity = entityDatabase.create();

  auto state = call(entity, "entity_query_get_first_by_name");
  EXPECT_TRUE(state["found_entity"].is<sol::nil_t>());
}

TEST_F(EntityQueryLuaTableTest,
       GetEntityByNameReturnsEntityTableIfEntityExists) {
  auto entity = entityDatabase.create();

  auto e1 = entityDatabase.create();
  entityDatabase.set<quoll::Name>(e1, {"Test"});

  auto state = call(entity, "entity_query_get_first_by_name");

  EXPECT_TRUE(state["found_entity"].is<quoll::EntityLuaTable>());
  EXPECT_EQ(state["found_entity"].get<quoll::EntityLuaTable>().getEntity(), e1);
}

TEST_F(EntityQueryLuaTableTest,
       DeleteEntityAddsDeleteComponentToExistingEntity) {
  auto entity = entityDatabase.create();

  call(entity, "entity_query_delete_entity");
  EXPECT_TRUE(entityDatabase.has<quoll::Delete>(entity));
}
