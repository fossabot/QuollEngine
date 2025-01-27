#pragma once

#include "quoll/asset/Asset.h"

namespace quoll {

/**
 * @brief Mesh component
 */
struct Mesh {
  /**
   * Mesh asset handle
   */
  MeshAssetHandle handle = MeshAssetHandle::Null;
};

} // namespace quoll
