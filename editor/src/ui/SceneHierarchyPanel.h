#pragma once

#include "scene/Scene.h"
#include "entity/EntityContext.h"
#include "../editor-scene/SceneManager.h"

namespace liquidator {

class SceneHierarchyPanel {
public:
  /**
   * @brief Create scene hierarchy panel
   *
   * @param context Entity context
   */
  SceneHierarchyPanel(liquid::EntityContext &context);

  /**
   * @brief Render the UI
   *
   * @param sceneManager Scene manager
   */
  void render(SceneManager &sceneManager);

private:
  /**
   * Render scene node as tree node
   *
   * @param sceneNode Scene node
   * @param flags Flags
   */
  void renderNode(liquid::SceneNode *node, int flags);

  /**
   * @brief Handle node deletion
   *
   * @param node Scene node
   */
  void handleDelete(liquid::SceneNode *node);

private:
  liquid::EntityContext &context;
};

} // namespace liquidator
