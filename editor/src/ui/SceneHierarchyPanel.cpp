#include "liquid/core/Base.h"
#include "SceneHierarchyPanel.h"
#include "ConfirmationDialog.h"

#include <imgui.h>
#include <glm/gtc/matrix_access.hpp>

namespace liquidator {

SceneHierarchyPanel::SceneHierarchyPanel(liquid::EntityContext &context_)
    : context(context_) {}

void SceneHierarchyPanel::render(SceneManager &sceneManager) {
  ImGui::Begin("Scene");
  for (auto *child :
       sceneManager.getActiveScene()->getRootNode()->getChildren()) {
    renderNode(child, ImGuiTreeNodeFlags_DefaultOpen, sceneManager);
  }

  ImGui::End();
}

void SceneHierarchyPanel::setNodeClickHandler(const NodeClickHandler &handler) {
  nodeClickHandler = handler;
}

void SceneHierarchyPanel::renderNode(liquid::SceneNode *node, int flags,
                                     SceneManager &sceneManager) {
  liquid::String name;
  if (context.hasComponent<liquid::NameComponent>(node->getEntity())) {
    name = context.getComponent<liquid::NameComponent>(node->getEntity()).name;
  } else {
    name = "Entity " + std::to_string(node->getEntity());
  }

  bool isLeaf = node->getChildren().empty();

  int treeNodeFlags = flags;
  if (isLeaf) {
    treeNodeFlags |=
        ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
  }

  if (selectedNode == node) {
    treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
  }

  bool open = false;
  if (ImGui::TreeNodeEx(name.c_str(), treeNodeFlags)) {
    open = !isLeaf;
    if (ImGui::IsItemClicked()) {
      nodeClickHandler(node);
      selectedNode = node;
    }
  }

  ConfirmationDialog confirmDeleteSceneNode(
      "Delete scene node#" + std::to_string(node->getEntity()),
      "Are you sure you want to delete node \"" + name + "\"?",
      [this, node](SceneManager &sceneManager) { handleDelete(node); },
      "Delete");

  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Go to view")) {
      handleMoveToNode(node, sceneManager.getEditorCamera());
    }

    if (ImGui::MenuItem("Delete")) {
      confirmDeleteSceneNode.show();
    }

    ImGui::EndPopup();
  }

  confirmDeleteSceneNode.render(sceneManager);

  if (open) {
    for (auto *child : node->getChildren()) {
      renderNode(child, 0, sceneManager);
    }
    ImGui::TreePop();
  }
}

void SceneHierarchyPanel::handleDelete(liquid::SceneNode *node) {
  auto entity = node->getEntity();
  auto *parent = node->getParent();

  if (parent) {
    parent->removeChild(node);
  }

  context.deleteEntity(entity);
}

void SceneHierarchyPanel::handleMoveToNode(liquid::SceneNode *node,
                                           EditorCamera &camera) {
  LIQUID_ASSERT(
      context.hasComponent<liquid::TransformComponent>(node->getEntity()),
      "Scene node must have transform component");

  auto &transformComponent =
      context.getComponent<liquid::TransformComponent>(node->getEntity());

  const auto &translation =
      glm::vec3(glm::column(transformComponent.worldTransform, 3));

  constexpr glm::vec3 distanceFromCenter{0.0f, 0.0f, 10.0f};

  camera.reset();
  camera.setCenter(translation);
  camera.setEye(translation - distanceFromCenter);
}

} // namespace liquidator
