#pragma once

#include "liquid/core/Property.h"

#include "liquid/rhi/RenderHandle.h"
#include "liquid/rhi/RenderDevice.h"
#include "liquid/rhi/Descriptor.h"

namespace liquid {

/**
 * @brief Material instance
 */
class Material {
public:
  /**
   * @brief Creates material
   *
   * @param textures Textures
   * @param properties Material properties
   * @param device Render device
   */
  Material(const std::vector<rhi::TextureHandle> &textures,
           const std::vector<std::pair<String, Property>> &properties,
           rhi::RenderDevice *device);

  /**
   * @brief Update property
   *
   * @param name Property name
   * @param value Property value
   */
  void updateProperty(StringView name, const Property &value);

  /**
   * @brief Get texture handles
   *
   * @return List of texture handles
   */
  inline const std::vector<rhi::TextureHandle> &getTextures() const {
    return mTextures;
  }

  /**
   * @brief Check if there are any textures
   *
   * @retval true Has textures
   * @retval false Does not have textures
   */
  inline bool hasTextures() const { return !mTextures.empty(); }

  /**
   * @brief Gets uniform buffer
   *
   * @return Uniform buffer
   */
  inline rhi::BufferHandle getBuffer() const { return mBuffer.getHandle(); }

  /**
   * @brief Get properties
   *
   * @return Unordered list of properties
   */
  inline const std::vector<Property> &getProperties() const {
    return mProperties;
  }

  /**
   * @brief Get descriptor
   *
   * @return rhi::Descriptor
   */
  inline const rhi::Descriptor &getDescriptor() const { return mDescriptor; }

private:
  /**
   * @brief Update buffer data
   *
   * Merges all properties into a memory region
   *
   * @return Size of buffer data
   */
  size_t updateBufferData();

private:
  std::vector<rhi::TextureHandle> mTextures;
  rhi::Buffer mBuffer;

  char *mData = nullptr;

  rhi::Descriptor mDescriptor;

  std::vector<Property> mProperties;
  std::map<String, size_t> mPropertyMap;
};

} // namespace liquid
