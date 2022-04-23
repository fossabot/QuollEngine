#pragma once

#include "liquid/asset/AssetManager.h"
#include "platform-tools/NativeFileDialog.h"

#include "../asset/GLTFImporter.h"
#include "IconRegistry.h"

#include "AssetLoadStatusDialog.h"

namespace liquidator {

class AssetBrowser {
  struct Entry {
    std::filesystem::path path;
    liquid::String clippedName;
    bool isDirectory = false;
    EditorIcon icon = EditorIcon::Unknown;
    liquid::AssetType assetType = liquid::AssetType::None;
    uint32_t asset = 0;
    bool isEditable = false;
  };

public:
  /**
   * @brief Create asset browser
   *
   * @param gltfImporter GLTF importer
   */
  AssetBrowser(GLTFImporter &gltfImporter);

  /**
   * @brief Render status bar
   *
   * @param assetManager Asset manager
   * @param iconRegistry Icon registry
   */
  void render(liquid::AssetManager &assetManager, IconRegistry &iconRegistry);

  /**
   * @brief Set on item open handler
   *
   * Called when item is opened (double
   * clicked) in the editor
   *
   * @param itemOpenHandler Item open handler
   */
  void setOnItemOpenHandler(
      const std::function<void(liquid::AssetType, uint32_t)> &itemOpenhandler);

  /**
   * @brief Reload contents in current directory
   */
  void reload();

private:
  /**
   * @brief Handle GLTF import
   */
  void handleGLTFImport();

  /**
   * @brief Handle entry creation
   */
  void handleCreateEntry();

private:
  Entry mStagingEntry;
  bool mHasStagingEntry = false;
  bool mInitialFocusSet = false;

  std::vector<Entry> mEntries;
  std::filesystem::path mCurrentDirectory;
  bool mDirectoryChanged = true;
  size_t mSelected = std::numeric_limits<size_t>::max();
  GLTFImporter &mGltfImporter;
  liquid::platform_tools::NativeFileDialog mFileDialog;

  std::function<void(liquid::AssetType, uint32_t)> mOnItemOpenHandler;

  AssetLoadStatusDialog mStatusDialog;
};

} // namespace liquidator