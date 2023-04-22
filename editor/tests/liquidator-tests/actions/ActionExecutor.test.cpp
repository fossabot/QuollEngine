#include "liquid/core/Base.h"
#include "liquidator/actions/ActionExecutor.h"

#include "liquidator-tests/Testing.h"

static const liquid::Path ScenePath{std::filesystem::current_path() / "scene"};

class ActionExecutorTest : public ::testing::Test {
public:
  void SetUp() override {
    std::filesystem::create_directories(ScenePath / "entities");

    YAML::Node scene;
    scene["persistentZone"] = 0;
    scene["zones"][0]["entities"] = (ScenePath / "entities").string();
    std::ofstream stream(ScenePath / "main.lqscene");
    stream << scene;
    stream.close();

    executor.getSceneIO().loadScene(ScenePath / "main.lqscene");
  }

  void TearDown() override { std::filesystem::remove_all(ScenePath); }

public:
  liquid::AssetRegistry registry;
  liquid::editor::WorkspaceState state{{}, registry};
  liquid::editor::ActionExecutor executor{state, ScenePath / "main.lqscene"};
};

struct TestActionData {
  bool called = false;
  std::vector<liquid::Entity> entitiesToSave;
  std::vector<liquid::Entity> entitiesToDelete;
  bool saveScene = false;
  bool mPredicate = true;
};

class TestAction : public liquid::editor::Action {
public:
  TestAction() : mData(new TestActionData) {}

  liquid::editor::ActionExecutorResult
  onExecute(liquid::editor::WorkspaceState &state) {
    mData->called = true;

    liquid::editor::ActionExecutorResult res{};

    res.entitiesToSave = mData->entitiesToSave;
    res.entitiesToDelete = mData->entitiesToDelete;
    res.saveScene = mData->saveScene;

    return res;
  }

  void saveEntityOnExecute(liquid::Entity entity) {
    mData->entitiesToSave.push_back(entity);
  }

  void deleteEntityOnExecute(liquid::Entity entity) {
    mData->entitiesToDelete.push_back(entity);
  }

  void saveSceneOnExecute() { mData->saveScene = true; }

  void setPredicate(bool predicate) { mData->mPredicate = predicate; }

  bool predicate(liquid::editor::WorkspaceState &state) {
    return mData->mPredicate;
  }

  inline liquid::SharedPtr<TestActionData> getData() { return mData; }

private:
  liquid::SharedPtr<TestActionData> mData;
};

TEST_F(ActionExecutorTest, ExecuteDoesNothingIfNoActionToProcess) {
  // Ensure that function does not
  // fail if there are no actions to
  // process
  executor.process();
}

TEST_F(ActionExecutorTest,
       ExecuteDoesNotCallActionExecutorIfActionPredicateReturnsFalse) {
  auto *actionPtr = new TestAction;
  actionPtr->setPredicate(false);

  auto actionData = actionPtr->getData();

  executor.execute(std::unique_ptr<liquid::editor::Action>(actionPtr));
  executor.process();
  EXPECT_FALSE(actionData->called);
}

TEST_F(ActionExecutorTest, ExecuteCallsActionExecutorWithState) {
  state.mode = liquid::editor::WorkspaceMode::Simulation;

  auto *actionPtr = new TestAction;
  auto actionData = actionPtr->getData();

  executor.execute(std::unique_ptr<liquid::editor::Action>(actionPtr));
  executor.process();
  EXPECT_TRUE(actionData->called);
}

TEST_F(ActionExecutorTest,
       ExecuteCreatesEntityFilesIfActionReturnsEntitiesToSaveAndModeIsEdit) {
  auto entity = state.scene.entityDatabase.create();
  state.scene.entityDatabase.set<liquid::Name>(entity, {"My name"});

  auto entityPath = ScenePath / "entities" / "1.lqnode";
  EXPECT_FALSE(std::filesystem::exists(entityPath));

  auto *actionPtr = new TestAction;
  actionPtr->saveEntityOnExecute(entity);
  auto actionData = actionPtr->getData();

  executor.execute(std::unique_ptr<liquid::editor::Action>(actionPtr));
  executor.process();
  EXPECT_TRUE(actionData->called);
  EXPECT_TRUE(std::filesystem::exists(entityPath));
}

TEST_F(
    ActionExecutorTest,
    ExecuteDoesNotCreateEntityFilesIfActionReturnsEntitiesToSaveAndModeIsSimulation) {
  state.mode = liquid::editor::WorkspaceMode::Simulation;

  auto entity = state.scene.entityDatabase.create();
  state.scene.entityDatabase.set<liquid::Name>(entity, {"My name"});

  auto entityPath = ScenePath / "entities" / "1.lqnode";
  EXPECT_FALSE(std::filesystem::exists(entityPath));

  auto *actionPtr = new TestAction;
  actionPtr->saveEntityOnExecute(entity);
  auto actionData = actionPtr->getData();

  executor.execute(std::unique_ptr<liquid::editor::Action>(actionPtr));
  executor.process();
  EXPECT_TRUE(actionData->called);
  EXPECT_FALSE(std::filesystem::exists(entityPath));
}

TEST_F(ActionExecutorTest,
       ExecuteDeletesEntityFilesIfActionReturnsEntitiesToDeleteAndModeIsEdit) {
  auto entity = state.scene.entityDatabase.create();
  state.scene.entityDatabase.set<liquid::Name>(entity, {"My name"});
  state.scene.entityDatabase.set<liquid::Id>(entity, {15});

  executor.getSceneIO().saveEntity(entity, ScenePath / "main.lqscene");

  auto entityPath = ScenePath / "entities" / "15.lqnode";
  EXPECT_TRUE(std::filesystem::exists(entityPath));

  auto *actionPtr = new TestAction;
  actionPtr->deleteEntityOnExecute(entity);
  auto actionData = actionPtr->getData();

  executor.execute(std::unique_ptr<liquid::editor::Action>(actionPtr));
  executor.process();
  EXPECT_TRUE(actionData->called);
  EXPECT_FALSE(std::filesystem::exists(entityPath));
}

TEST_F(
    ActionExecutorTest,
    ExecuteDoesNotDeleteEntityFilesIfActionReturnsEntitiesToDeleteAndModeIsSimulation) {
  state.mode = liquid::editor::WorkspaceMode::Simulation;

  auto entity = state.scene.entityDatabase.create();
  state.scene.entityDatabase.set<liquid::Name>(entity, {"My name"});
  state.scene.entityDatabase.set<liquid::Id>(entity, {15});

  executor.getSceneIO().saveEntity(entity, ScenePath / "main.lqscene");

  auto entityPath = ScenePath / "entities" / "15.lqnode";
  EXPECT_TRUE(std::filesystem::exists(entityPath));

  auto *actionPtr = new TestAction;
  actionPtr->deleteEntityOnExecute(entity);
  auto actionData = actionPtr->getData();

  executor.execute(std::unique_ptr<liquid::editor::Action>(actionPtr));
  executor.process();
  EXPECT_TRUE(actionData->called);
  EXPECT_TRUE(std::filesystem::exists(entityPath));
}

TEST_F(ActionExecutorTest,
       ExecuteSavesSceneFileIfActionReturnsSaveSceneAndModeIsEdit) {
  auto entity = state.scene.entityDatabase.create();
  state.scene.entityDatabase.set<liquid::PerspectiveLens>(entity, {});
  state.scene.entityDatabase.set<liquid::Id>(entity, {15});
  state.scene.activeCamera = entity;

  executor.getSceneIO().saveEntity(entity, ScenePath / "main.lqscene");

  auto entityPath = ScenePath / "entities" / "15.lqnode";
  EXPECT_TRUE(std::filesystem::exists(entityPath));

  auto *actionPtr = new TestAction;
  actionPtr->saveSceneOnExecute();
  auto actionData = actionPtr->getData();

  executor.execute(std::unique_ptr<liquid::editor::Action>(actionPtr));
  executor.process();
  EXPECT_TRUE(actionData->called);

  std::ifstream stream(ScenePath / "main.lqscene");
  auto node = YAML::Load(stream);
  stream.close();

  auto startingCamera =
      node["zones"][node["persistentZone"].as<size_t>()]["startingCamera"];
  EXPECT_TRUE(startingCamera);
  EXPECT_TRUE(startingCamera.IsScalar());
  EXPECT_EQ(startingCamera.as<uint32_t>(), 15);
}

TEST_F(ActionExecutorTest,
       ExecuteDoesNotSaveSceneFileIfActionReturnsSaveSceneAndModeIsSimulation) {
  state.mode = liquid::editor::WorkspaceMode::Simulation;

  auto entity = state.scene.entityDatabase.create();
  state.scene.entityDatabase.set<liquid::PerspectiveLens>(entity, {});
  state.scene.entityDatabase.set<liquid::Id>(entity, {15});
  state.scene.activeCamera = entity;

  executor.getSceneIO().saveEntity(entity, ScenePath / "main.lqscene");

  auto entityPath = ScenePath / "entities" / "15.lqnode";
  EXPECT_TRUE(std::filesystem::exists(entityPath));

  auto *actionPtr = new TestAction;
  actionPtr->saveSceneOnExecute();
  auto actionData = actionPtr->getData();

  executor.execute(std::unique_ptr<liquid::editor::Action>(actionPtr));
  executor.process();
  EXPECT_TRUE(actionData->called);

  std::ifstream stream(ScenePath / "main.lqscene");
  auto node = YAML::Load(stream);
  stream.close();

  auto startingCamera =
      node["zones"][node["persistentZone"].as<size_t>()]["startingCamera"];
  EXPECT_FALSE(startingCamera);
}
