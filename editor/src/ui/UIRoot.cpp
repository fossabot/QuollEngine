#include <iostream>
#include "UIRoot.h"

namespace liquidator {

liquidator::UIRoot::UIRoot(liquid::EntityContext &context,
                           const liquid::TinyGLTFLoader &gltfLoader)
    : menuBar(gltfLoader), sceneHierarchyPanel(context), entityPanel(context) {
  sceneHierarchyPanel.setNodeClickHandler(
      [this](liquid::SceneNode *node) { handleNodeClick(node); });
}

void liquidator::UIRoot::render(SceneManager &sceneManager) {
  menuBar.render(sceneManager);
  sceneHierarchyPanel.render(sceneManager);
  entityPanel.render(sceneManager);
}

void UIRoot::handleNodeClick(liquid::SceneNode *node) {
  entityPanel.setSelectedEntity(node->getEntity());
}

} // namespace liquidator
