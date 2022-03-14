#pragma once

#include "liquid/core/Property.h"

#include "liquid/rhi/RenderHandle.h"
#include "liquid/rhi/ResourceRegistry.h"
#include "liquid/rhi/Descriptor.h"

namespace liquid {

class Material {
public:
  /**
   * @brief Creates material
   *
   * @param textures Textures
   * @param properties Material properties
   * @param registry Resource registry
   */
  Material(const std::vector<rhi::TextureHandle> &textures,
           const std::vector<std::pair<String, Property>> &properties,
           rhi::ResourceRegistry &registry);

  /**
   * @brief Update property
   *
   * @param name Property name
   * @param property Property value
   */
  void updateProperty(const String &name, const Property &value);

  /**
   * @brief Get texture handles
   *
   * @return List of texture handles
   */
  inline const std::vector<rhi::TextureHandle> &getTextures() const {
    return textures;
  }

  /**
   * @brief Check if there are any textures
   *
   * @retval true Has textures
   * @retval false Does not have textures
   */
  inline bool hasTextures() const { return !textures.empty(); }

  /**
   * @brief Gets uniform buffer
   *
   * @return Uniform buffer
   */
  inline rhi::BufferHandle getUniformBuffer() const { return uniformBuffer; }

  /**
   * @brief Get properties
   *
   * @return Unordered list of properties
   */
  inline const std::vector<Property> &getProperties() const {
    return properties;
  }

  /**
   * @brief Get descriptor
   *
   * @return rhi::Descriptor
   */
  inline const rhi::Descriptor &getDescriptor() const { return descriptor; }

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
  std::vector<rhi::TextureHandle> textures;
  rhi::BufferHandle uniformBuffer = rhi::BufferHandle::Invalid;

  rhi::ResourceRegistry &registry;
  char *data = nullptr;

  rhi::Descriptor descriptor;

  std::vector<Property> properties;
  std::map<String, size_t> propertyMap;
};

} // namespace liquid
