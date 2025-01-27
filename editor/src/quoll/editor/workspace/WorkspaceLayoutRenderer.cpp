#include "quoll/core/Base.h"
#include "quoll/imgui/Imgui.h"

#include "quoll/editor/ui/Toolbar.h"

#include "WorkspaceLayoutRenderer.h"

namespace quoll::editor {

bool WorkspaceLayoutRenderer::begin() {
  const f32 WINDOW_AND_STATUS_BAR_HEIGHT = ImGui::GetFrameHeight() * 2.0f;

  const auto &viewport = ImGui::GetMainViewport();

  if (viewport->Size.x <= 0.0f || viewport->Size.y <= 0.0f)
    return false;

  ImGui::SetNextWindowPos(
      ImVec2(viewport->Pos.x,
             viewport->Pos.y + ImGui::GetFrameHeight() + Toolbar::Height));
  ImGui::SetNextWindowSize(ImVec2(
      // Extract status bar from viewport size
      viewport->Size.x,
      viewport->Size.y - WINDOW_AND_STATUS_BAR_HEIGHT - Toolbar::Height));
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("QuollEditorDockspaceMain", nullptr, flags);
  ImGui::PopStyleVar();
  ImGui::PopStyleVar(2);

  auto dockspaceId = ImGui::GetID("QuollEditorDockspace");
  ImGui::DockSpace(dockspaceId, ImVec2{0.0f, 0.0f},
                   ImGuiDockNodeFlags_PassthruCentralNode);

  return true;
}

void WorkspaceLayoutRenderer::end() { ImGui::End(); }

void WorkspaceLayoutRenderer::reset() {
  const auto &viewport = ImGui::GetMainViewport();
  auto dockspaceId = ImGui::GetID("QuollEditorDockspace");

  ImGui::DockBuilderRemoveNode(dockspaceId);
  ImGui::DockBuilderAddNode(
      dockspaceId,
      // Imgui dockspace two enums that can
      // be used together. Linter does not like performing
      // bitwise operations on this; so, supressing the warning
      // just for this special use-case
      // NOLINTNEXTLINE(clang-diagnostic-deprecated-enum-enum-conversion)
      ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
  ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->Size);

  // Default template
  // Sidebar - Hierarchy (top) and Inspector (bottom)
  // Main - View (top) and Asset browser (bottom)

  static constexpr f32 RatioSidebar = 1.0f / 6.0f;
  static constexpr f32 RatioMainBottom = 1.0f / 4.0f;
  static constexpr f32 RatioSideBottom = 2.0f / 3.0f;

  ImGuiID mainId = -1;
  auto sidebarId = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right,
                                               RatioSidebar, nullptr, &mainId);

  ImGuiID sidebarTopId = -1;
  auto sidebarBottomId = ImGui::DockBuilderSplitNode(
      sidebarId, ImGuiDir_Down, RatioSideBottom, nullptr, &sidebarTopId);

  ImGuiID mainTopId = -1;
  auto mainBottomId = ImGui::DockBuilderSplitNode(
      mainId, ImGuiDir_Down, RatioMainBottom, nullptr, &mainTopId);

  ImGui::DockBuilderDockWindow("Scene", sidebarTopId);
  ImGui::DockBuilderDockWindow("Inspector", sidebarBottomId);
  ImGui::DockBuilderDockWindow("View", mainTopId);
  ImGui::DockBuilderDockWindow("Asset Browser", mainBottomId);
  ImGui::DockBuilderDockWindow("Logs", mainBottomId);

  ImGui::DockBuilderFinish(dockspaceId);
}

void WorkspaceLayoutRenderer::resize() {
  const auto &viewport = ImGui::GetMainViewport();
  auto dockspaceId = ImGui::GetID("QuollEditorDockspace");
  ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->Size);
}

} // namespace quoll::editor
