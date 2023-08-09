#include "liquid/schemas/generated/Base.schema.h"

namespace liquid::schemas {

static base::Vec3 toFb(const glm::vec3 &value) {
  return {value.x, value.y, value.z};
}

static base::Vec4 toFb(const glm::vec4 &value) {
  return {value.x, value.y, value.z, value.w};
}

static base::Quat toFb(const glm::quat &value) {
  return {value.x, value.y, value.z, value.w};
}

static base::Mat4 toFb(const glm::mat4 &value) {
  return {std::array<base::Vec4, 4>{toFb(value[0]), toFb(value[1]),
                                    toFb(value[2]), toFb(value[3])}};
}

static glm::vec3 fromFb(const base::Vec3 *value) {
  return {value->x(), value->y(), value->z()};
}

static glm::vec4 fromFb(const base::Vec4 *value) {
  return {value->x(), value->y(), value->z(), value->w()};
}

static glm::quat fromFb(const base::Quat *value) {
  return {value->w(), value->x(), value->y(), value->z()};
}

static glm::mat4 fromFb(const base::Mat4 *value) {
  return {fromFb(value->value()->Get(0)), fromFb(value->value()->Get(1)),
          fromFb(value->value()->Get(2)), fromFb(value->value()->Get(3))};
}

static String fromFb(const flatbuffers::String *value) { return value->str(); }

namespace detail {

template <typename TFB, typename TFrom>
static std::vector<TFB> vectorToFb(const std::vector<TFrom> &value) {
  std::vector<TFB> output(value.size());
  for (size_t i = 0; i < output.size(); ++i) {
    output.at(i) = toFb(value.at(i));
  }

  return output;
}

template <typename TFrom, typename TFB>
static std::vector<TFrom>
vectorFromFb(const flatbuffers::Vector<const TFB *> *value) {
  std::vector<TFrom> output(value->size());
  for (size_t i = 0; i < output.size(); ++i) {
    output.at(i) = fromFb(value->Get(static_cast<flatbuffers::uoffset_t>(i)));
  }

  return output;
}

} // namespace detail

static std::vector<base::Vec3> toFb(const std::vector<glm::vec3> &value) {
  return detail::vectorToFb<base::Vec3>(value);
}

static std::vector<base::Vec4> toFb(const std::vector<glm::vec4> &value) {
  return detail::vectorToFb<base::Vec4>(value);
}

static std::vector<base::Quat> toFb(const std::vector<glm::quat> &value) {
  return detail::vectorToFb<base::Quat>(value);
}

static std::vector<base::Mat4> toFb(const std::vector<glm::mat4> &value) {
  return detail::vectorToFb<base::Mat4>(value);
}

static std::vector<glm::vec3>
fromFb(const flatbuffers::Vector<const base::Vec3 *> *value) {
  return detail::vectorFromFb<glm::vec3>(value);
}

static std::vector<glm::vec4>
fromFb(const flatbuffers::Vector<const base::Vec4 *> *value) {
  return detail::vectorFromFb<glm::vec4>(value);
}

static std::vector<glm::quat>
fromFb(const flatbuffers::Vector<const base::Quat *> *value) {
  return detail::vectorFromFb<glm::quat>(value);
}

static std::vector<glm::mat4>
fromFb(const flatbuffers::Vector<const base::Mat4 *> *value) {
  return detail::vectorFromFb<glm::mat4>(value);
}

static std::vector<String>
fromFb(const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>
           *value) {
  std::vector<String> output(value->size());
  for (size_t i = 0; i < output.size(); ++i) {
    output.at(i) =
        fromFb(value->GetAsString(static_cast<flatbuffers::uoffset_t>(i)));
  }

  return output;
}

static std::vector<uint8_t> fromFb(const flatbuffers::Vector<uint8_t> *value) {
  std::vector<uint8_t> output(value->size());
  for (size_t i = 0; i < output.size(); ++i) {
    output.at(i) = value->Get(static_cast<flatbuffers::uoffset_t>(i));
  }

  return output;
}

} // namespace liquid::schemas
