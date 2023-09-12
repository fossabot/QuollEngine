#pragma once

#include "NativeDescriptor.h"

namespace quoll::rhi {

/**
 * @brief Descriptor
 *
 * Proxies all calls to native descriptor
 */
class Descriptor {
public:
  /**
   * @brief Create descriptor
   */
  Descriptor() = default;

  /**
   * @brief Create descriptor
   *
   * @param nativeDescriptor Native descriptor
   * @param handle Descriptor handle
   */
  Descriptor(NativeDescriptor *nativeDescriptor, DescriptorHandle handle);

  /**
   * @brief Bind texture descriptors
   *
   * @param binding Binding number
   * @param textures Textures
   * @param type Descriptor type
   * @param start Starting index
   * @return Current object
   */
  Descriptor &write(uint32_t binding, std::span<TextureHandle> textures,
                    DescriptorType type, uint32_t start = 0);

  /**
   * @brief Bind sampler descriptors
   *
   * @param binding Binding number
   * @param samplers Samplers
   * @param start Starting index
   * @return Current object
   */
  Descriptor &write(uint32_t binding, std::span<SamplerHandle> samplers,
                    uint32_t start = 0);

  /**
   * @brief Bind buffer descriptors
   *
   * @param binding Binding number
   * @param buffers Buffers
   * @param type Descriptor type
   * @param start Starting index
   * @return Current object
   */
  Descriptor &write(uint32_t binding, std::span<BufferHandle> buffers,
                    DescriptorType type, uint32_t start = 0);

  /**
   * @brief Bind buffer descriptors
   *
   * @param binding Binding number
   * @param bufferInfos Buffer infos
   * @param type Descriptor type
   * @param start Starting index
   * @return Current object
   */
  Descriptor &write(uint32_t binding,
                    std::span<DescriptorBufferInfo> bufferInfos,
                    DescriptorType type, uint32_t start = 0);

  /**
   * @brief Get descriptor handle
   *
   * @return Descriptor handle
   */
  inline DescriptorHandle getHandle() const { return mHandle; }

private:
  NativeDescriptor *mNativeDescriptor = nullptr;
  DescriptorHandle mHandle{0};
};

} // namespace quoll::rhi