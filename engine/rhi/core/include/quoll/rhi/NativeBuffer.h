#pragma once

#include "quoll/rhi/DeviceAddress.h"

namespace quoll::rhi {

/**
 * @brief Native buffer interface
 */
class NativeBuffer {
public:
  /**
   * @brief Default destructor
   */
  virtual ~NativeBuffer() = default;

  /**
   * @brief Map buffer
   *
   * @return Mapped data
   */
  virtual void *map() = 0;

  /**
   * @brief Unmap buffer
   */
  virtual void unmap() = 0;

  /**
   * @brief Resize buffer
   *
   * Recreates the buffer with
   * new size. Does not retain
   * the previous data in it
   *
   * @param size New size
   */
  virtual void resize(usize size) = 0;

  /**
   * @brief Get buffer device address
   *
   * @return Device address
   */
  virtual DeviceAddress getAddress() = 0;
};

} // namespace quoll::rhi
