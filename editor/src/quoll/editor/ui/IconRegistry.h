#pragma once

#include "quoll/rhi/RenderHandle.h"
#include "quoll/rhi/RenderDevice.h"
#include "quoll/renderer/RenderStorage.h"

namespace quoll::editor {

enum class EditorIcon {
  Unknown,
  Directory,
  Material,
  Texture,
  Font,
  Mesh,
  SkinnedMesh,
  Skeleton,
  Animation,
  Animator,
  InputMap,
  Audio,
  Prefab,
  LuaScript,
  Sun,
  Light,
  Camera,
  Environment,
  Scene
};

/**
 * @brief Icon registry
 *
 * Provides a way to select and render
 * icons
 */
class IconRegistry {
public:
  /**
   * @brief Load icons from path
   *
   * @param renderStorage Render storage
   * @param iconsPath Path to icons
   */
  static void loadIcons(RenderStorage &renderStorage,
                        const std::filesystem::path &iconsPath);

  /**
   * @brief Get icon
   *
   * @param icon Icon enum
   * @return Texture handle for the icon
   */
  static inline rhi::TextureHandle getIcon(EditorIcon icon) {
    return mIconMap.at(icon);
  }

private:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables,-warnings-as-errors)
  static std::unordered_map<EditorIcon, rhi::TextureHandle> mIconMap;
};

} // namespace quoll::editor
