#include "quoll/core/Base.h"
#include "quoll/core/Engine.h"
#include "Material.h"

namespace quoll {

Material::Material(const String &name,
                   const std::vector<rhi::TextureHandle> &textures,
                   const std::vector<std::pair<String, Property>> &properties,
                   RenderStorage &renderStorage)
    : mTextures(textures) {

  for (usize i = 0; i < properties.size(); ++i) {
    auto &prop = properties[i];
    mProperties.push_back(prop.second);
    mPropertyMap.insert({prop.first, i});
  }

  if (!mProperties.empty()) {
    auto size = updateBufferData();
    rhi::BufferDescription desc{rhi::BufferUsage::Uniform, size, mData};
    desc.debugName = name;
    mBuffer = renderStorage.createBuffer(desc);
  }
}

void Material::updateProperty(StringView name, const Property &value) {
  const auto &it = mPropertyMap.find(String(name));
  if (it == mPropertyMap.end() || (*it).second > mProperties.size()) {
    Engine::getLogger().warning()
        << "Property \"" << name
        << "\" does not exist in material. Skipping...";
    return;
  }

  usize index = (*it).second;

  if (mProperties.at(index).getType() != value.getType()) {
    Engine::getLogger().warning()
        << "Type of property \"" << name
        << "\" does match the type of new property. Skipping...";
    return;
  }

  mProperties.at(index) = value;
  auto size = updateBufferData();
  mBuffer.update(mData, size);
}

usize Material::updateBufferData() {
  if (mData) {
    delete mData;
  }

  usize size = 0;
  usize idx = 0;

  usize maxValue = 0;
  for (auto &value : mProperties) {
    maxValue = maxValue > value.getSize() ? maxValue : value.getSize();
  }

  size = maxValue * mProperties.size();

  mData = new char[size];

  for (auto &value : mProperties) {
    if (value.getType() == Property::INT32) {
      auto val = value.getValue<i32>();
      memcpy(mData + idx, &val, maxValue);
    } else if (value.getType() == Property::UINT32) {
      auto val = value.getValue<u32>();
      memcpy(mData + idx, &val, maxValue);
    } else if (value.getType() == Property::REAL) {
      auto val = value.getValue<f32>();
      memcpy(mData + idx, &val, maxValue);
    } else if (value.getType() == Property::VECTOR2) {
      auto &val = value.getValue<glm::vec2>();
      memcpy(mData + idx, &val, maxValue);
    } else if (value.getType() == Property::VECTOR3) {
      auto &val = value.getValue<glm::vec3>();
      memcpy(mData + idx, &val, maxValue);
    } else if (value.getType() == Property::VECTOR4) {
      auto &val = value.getValue<glm::vec4>();
      memcpy(mData + idx, &val, maxValue);
    } else if (value.getType() == Property::MATRIX4) {
      auto &val = value.getValue<glm::mat4>();
      memcpy(mData + idx, &val, maxValue);
    }
    idx += maxValue;
  }

  return size;
}

} // namespace quoll
