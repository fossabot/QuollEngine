#pragma once

#include "liquidator/state/WorkspaceState.h"
#include "liquidator/actions/ActionExecutor.h"
#include "liquidator/actions/EntityTransformActions.h"

namespace liquid::editor {

/**
 * @brief Scene gizmos
 *
 * Renders and controls gizmos in the scene
 */
class SceneGizmos {
public:
  /**
   * @brief Render scene gizmos
   *
   * @param state Workspace state
   * @param actionExecutor Action executor
   * @retval true Gizmo is hovered
   * @retval false Gizmo is not hovered
   */
  bool render(WorkspaceState &state, ActionExecutor &actionExecutor);

private:
  std::unique_ptr<EntitySetLocalTransformContinuous> mAction;
};

} // namespace liquid::editor