#pragma once

#include "Action.h"

#include "quoll/renderer/MeshRenderer.h"

namespace quoll::editor {

/**
 * @brief Set material slot for mesh renderer
 */
class EntitySetMeshRendererMaterial : public Action {
public:
  /**
   * @brief Create action
   *
   * @param entity Entity
   * @param slot Material slot
   * @param handle New material
   */
  EntitySetMeshRendererMaterial(Entity entity, usize slot,
                                MaterialAssetHandle handle);

  /**
   * @brief Action executor
   *
   * @param state Workspace state
   * @param assetRegistry Asset registry
   * @return Executor result
   */
  ActionExecutorResult onExecute(WorkspaceState &state,
                                 AssetRegistry &assetRegistry) override;

  /**
   * @brief Action undo
   *
   * @param state Workspace state
   * @param assetRegistry Asset registry
   * @return Executor result
   */
  ActionExecutorResult onUndo(WorkspaceState &state,
                              AssetRegistry &assetRegistry) override;

  /**
   * @brief Action predicate
   *
   * @param state Workspace state
   * @param assetRegistry Asset registry
   * @retval true Predicate is true
   * @retval false Predicate is false
   */
  bool predicate(WorkspaceState &state, AssetRegistry &assetRegistry) override;

private:
  Entity mEntity;
  usize mSlot;
  MaterialAssetHandle mOldMaterial = MaterialAssetHandle::Null;
  MaterialAssetHandle mNewMaterial;
};

/**
 * @brief Add new material slot for mesh renderer
 */
class EntityAddMeshRendererMaterialSlot : public Action {
public:
  /**
   * @brief Create action
   *
   * @param entity Entity
   * @param handle New material
   */
  EntityAddMeshRendererMaterialSlot(Entity entity, MaterialAssetHandle handle);

  /**
   * @brief Action executor
   *
   * @param state Workspace state
   * @param assetRegistry Asset registry
   * @return Executor result
   */
  ActionExecutorResult onExecute(WorkspaceState &state,
                                 AssetRegistry &assetRegistry) override;

  /**
   * @brief Action undo
   *
   * @param state Workspace state
   * @param assetRegistry Asset registry
   * @return Executor result
   */
  ActionExecutorResult onUndo(WorkspaceState &state,
                              AssetRegistry &assetRegistry) override;

  /**
   * @brief Action predicate
   *
   * @param state Workspace state
   * @param assetRegistry Asset registry
   * @retval true Predicate is true
   * @retval false Predicate is false
   */
  bool predicate(WorkspaceState &state, AssetRegistry &assetRegistry) override;

private:
  Entity mEntity;
  MaterialAssetHandle mNewMaterial;
};

/**
 * @brief Remove last mesh renderer material slot
 */
class EntityRemoveLastMeshRendererMaterialSlot : public Action {
public:
  /**
   * @brief Create action
   *
   * @param entity Entity
   */
  EntityRemoveLastMeshRendererMaterialSlot(Entity entity);

  /**
   * @brief Action executor
   *
   * @param state Workspace state
   * @param assetRegistry Asset registry
   * @return Executor result
   */
  ActionExecutorResult onExecute(WorkspaceState &state,
                                 AssetRegistry &assetRegistry) override;

  /**
   * @brief Action undo
   *
   * @param state Workspace state
   * @param assetRegistry Asset registry
   * @return Executor result
   */
  ActionExecutorResult onUndo(WorkspaceState &state,
                              AssetRegistry &assetRegistry) override;

  /**
   * @brief Action predicate
   *
   * @param state Workspace state
   * @param assetRegistry Asset registry
   * @retval true Predicate is true
   * @retval false Predicate is false
   */
  bool predicate(WorkspaceState &state, AssetRegistry &assetRegistry) override;

private:
  Entity mEntity;
  MaterialAssetHandle mOldMaterial = MaterialAssetHandle::Null;
};

} // namespace quoll::editor
