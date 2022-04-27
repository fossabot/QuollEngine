#include "liquid/core/Base.h"
#include "ImguiUtils.h"

namespace liquid::imgui {

inline ImTextureID getImguiTexture(liquid::rhi::TextureHandle handle) {
  return reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(handle));
}

void image(liquid::rhi::TextureHandle handle, const ImVec2 &size,
           const ImVec2 &uv0, const ImVec2 &uv1, const ImVec4 &tint_col,
           const ImVec4 &border_col) {
  ImGui::Image(getImguiTexture(handle), size, uv0, uv1, tint_col, border_col);
}

bool imageButton(liquid::rhi::TextureHandle handle, const ImVec2 &size,
                 const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding,
                 const ImVec4 &bg_col, const ImVec4 &tint_col) {
  return ImGui::ImageButton(getImguiTexture(handle), size, uv0, uv1,
                            frame_padding, bg_col, tint_col);
}

void renderColumn(const glm::vec3 &value) {
  ImGui::TableNextColumn();
  ImGui::Text("%.2f %.2f %.2f", value.x, value.y, value.z);
}

void renderColumn(const glm::quat &value) {
  ImGui::TableNextColumn();
  ImGui::Text("%.2f %.2f %.2f %.2f", value.x, value.y, value.z, value.w);
}

void renderColumn(float value) {
  ImGui::TableNextColumn();
  ImGui::Text("%.2f", value);
}

void renderColumn(const String &value) {
  ImGui::TableNextColumn();
  ImGui::Text("%s", value.c_str());
}

void renderColumn(uint32_t value) {
  ImGui::TableNextColumn();
  ImGui::Text("%d", value);
}

} // namespace liquid::imgui