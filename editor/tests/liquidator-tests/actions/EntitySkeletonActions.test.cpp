#include "liquid/core/Base.h"
#include "liquidator/actions/EntitySkeletonActions.h"

#include "liquidator-tests/Testing.h"
#include "ActionTestBase.h"
#include "DefaultEntityTests.h"

using EntityToggleSkeletonDebugBonesActionTest = ActionTestBase;

TEST_P(EntityToggleSkeletonDebugBonesActionTest,
       ExecutorSetsDebugBonesForEntityIfNoDebugBones) {
  auto entity = activeScene().entityDatabase.create();
  activeScene().entityDatabase.set<liquid::Skeleton>(entity, {});

  liquid::editor::EntityToggleSkeletonDebugBones action(entity);
  auto res = action.onExecute(state);

  EXPECT_TRUE(activeScene().entityDatabase.has<liquid::SkeletonDebug>(entity));
  EXPECT_TRUE(res.entitiesToSave.empty());
}

TEST_P(EntityToggleSkeletonDebugBonesActionTest,
       ExecutorRemovesDebugBonesForEntityIfHasDebugBones) {
  auto entity = activeScene().entityDatabase.create();
  activeScene().entityDatabase.set<liquid::Skeleton>(entity, {});
  activeScene().entityDatabase.set<liquid::SkeletonDebug>(entity, {});

  liquid::editor::EntityToggleSkeletonDebugBones action(entity);
  auto res = action.onExecute(state);

  EXPECT_FALSE(activeScene().entityDatabase.has<liquid::SkeletonDebug>(entity));
  EXPECT_TRUE(res.entitiesToSave.empty());
}

TEST_P(EntityToggleSkeletonDebugBonesActionTest,
       PredicateReturnsFalseIfEntityHasNoSkeleton) {
  auto entity = activeScene().entityDatabase.create();

  liquid::editor::EntityToggleSkeletonDebugBones action(entity);
  EXPECT_FALSE(action.predicate(state));
}

TEST_P(EntityToggleSkeletonDebugBonesActionTest,
       PredicateReturnsTrueIfEntityHasSkeleton) {
  auto entity = activeScene().entityDatabase.create();
  activeScene().entityDatabase.set<liquid::Skeleton>(entity, {});

  liquid::editor::EntityToggleSkeletonDebugBones action(entity);
  EXPECT_TRUE(action.predicate(state));
}

InitActionsTestSuite(EntityActionsTest,
                     EntityToggleSkeletonDebugBonesActionTest);

using EntityDeleteSkeletonActionTest = ActionTestBase;

InitDefaultDeleteComponentTests(EntityDeleteSkeletonActionTest,
                                EntityDeleteSkeleton, Skeleton);

TEST_P(EntityDeleteSkeletonActionTest,
       ExecutorDeletesSkeletonDebugComponentFromEntity) {
  auto entity = activeScene().entityDatabase.create();
  activeScene().entityDatabase.set<liquid::Skeleton>(entity, {});
  activeScene().entityDatabase.set<liquid::SkeletonDebug>(entity, {});

  liquid::editor::EntityDeleteSkeleton action(entity);
  auto res = action.onExecute(state);

  EXPECT_FALSE(activeScene().entityDatabase.has<liquid::Skeleton>(entity));
  EXPECT_FALSE(activeScene().entityDatabase.has<liquid::SkeletonDebug>(entity));
  ASSERT_EQ(res.entitiesToSave.size(), 1);
  EXPECT_EQ(res.entitiesToSave.at(0), entity);
}

TEST_P(
    EntityDeleteSkeletonActionTest,
    UndoDoesNotCreateSkeletonDebugComponentForEntityIfItDidNotExistDuringExecution) {
  auto entity = activeScene().entityDatabase.create();
  activeScene().entityDatabase.set<liquid::Skeleton>(entity, {});

  liquid::editor::EntityDeleteSkeleton action(entity);
  action.onExecute(state);
  auto res = action.onUndo(state);

  EXPECT_FALSE(activeScene().entityDatabase.has<liquid::SkeletonDebug>(entity));
  ASSERT_EQ(res.entitiesToSave.size(), 1);
  EXPECT_EQ(res.entitiesToSave.at(0), entity);
}

TEST_P(EntityDeleteSkeletonActionTest,
       UndoCretesSkeletonDebugComponentForEntityIfItExistedDuringExecution) {
  auto entity = activeScene().entityDatabase.create();
  activeScene().entityDatabase.set<liquid::Skeleton>(entity, {});
  activeScene().entityDatabase.set<liquid::SkeletonDebug>(entity, {});

  liquid::editor::EntityDeleteSkeleton action(entity);
  action.onExecute(state);

  auto res = action.onUndo(state);

  EXPECT_TRUE(activeScene().entityDatabase.has<liquid::SkeletonDebug>(entity));
  ASSERT_EQ(res.entitiesToSave.size(), 1);
  EXPECT_EQ(res.entitiesToSave.at(0), entity);
}

InitActionsTestSuite(EntityActionsTest, EntityDeleteSkeletonActionTest);
