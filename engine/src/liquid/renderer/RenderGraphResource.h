#pragma once

#include "RenderGraphRegistry.h"

namespace liquid {

/**
 * @brief Render graph resource
 *
 * @tparam THandle Real resource handle
 */
template <class THandle> class RenderGraphResource {
public:
  /**
   * @brief Create render graph resource
   *
   * @param registry Render graph registry
   * @param index Resource index
   */
  RenderGraphResource(RenderGraphRegistry &registry, size_t index)
      : mRegistry(registry), mIndex(index) {}

  /**
   * @brief Get real resource handle
   *
   * @return Real resource handle
   */
  inline operator THandle() const { return getHandle(); }

  /**
   * @brief Get real resource handle
   *
   * @return Real resource handle
   */
  inline THandle getHandle() const { return mRegistry.get<THandle>(mIndex); }

private:
  RenderGraphRegistry &mRegistry;
  size_t mIndex;
};

} // namespace liquid
