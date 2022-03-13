#pragma once

#include "liquid/core/Property.h"
#include "Shader.h"
#include "MaterialResourceBinder.h"
#include "liquid/rhi/Descriptor.h"

#include "liquid/rhi/RenderHandle.h"
#include "liquid/rhi/ResourceRegistry.h"

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
  Material(const std::vector<TextureHandle> &textures,
           const std::vector<std::pair<String, Property>> &properties,
           experimental::ResourceRegistry &registry);

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
  inline const std::vector<TextureHandle> &getTextures() const {
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
   * @brief Gets vertex shader
   *
   * @return Pointer to vertex shader
   */
  inline const SharedPtr<Shader> &getVertexShader() { return vertexShader; }

  /**
   * @brief Gets fragment shader
   *
   * @return Pointer to fragment shader
   */
  inline const SharedPtr<Shader> &getFragmentShader() { return fragmentShader; }

  /**
   * @brief Gets uniform buffer
   *
   * @return Uniform buffer
   */
  inline BufferHandle getUniformBuffer() const { return uniformBuffer; }

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
   * @return Descriptor
   */
  inline const Descriptor &getDescriptor() const { return descriptor; }

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
  SharedPtr<Shader> vertexShader = nullptr;
  SharedPtr<Shader> fragmentShader = nullptr;
  std::vector<TextureHandle> textures;
  BufferHandle uniformBuffer = 0;

  experimental::ResourceRegistry &registry;
  char *data = nullptr;

  Descriptor descriptor;

  std::vector<Property> properties;
  std::map<String, size_t> propertyMap;
};

} // namespace liquid