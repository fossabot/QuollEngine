#pragma once

namespace quoll {

/**
 * @brief Describes material attached to a shape
 */
struct PhysicsMaterialDesc {
  /**
   * Static friction
   */
  f32 staticFriction = 0.0f;

  /**
   * Dynamic friction
   */
  f32 dynamicFriction = 0.0f;

  /**
   * Restitution
   */
  f32 restitution = 1.0f;
};

/**
 * @brief Geometry types
 */
enum class PhysicsGeometryType { Box, Sphere, Capsule, Plane };

/**
 * @brief Get physics geometry type string
 *
 * @param type Physics geometry type
 * @return Type string
 */
inline String getPhysicsGeometryTypeString(PhysicsGeometryType type) {
  switch (type) {
  case PhysicsGeometryType::Box:
    return "box";
  case PhysicsGeometryType::Sphere:
    return "sphere";
  case PhysicsGeometryType::Capsule:
    return "capsule";
  case PhysicsGeometryType::Plane:
    return "plane";
  default:
    return "unknown";
  }
}

/**
 * @brief Sphere geometry data
 */
struct PhysicsGeometrySphere {
  /**
   * Sphere radius
   */
  f32 radius = 1.0f;
};

/**
 * @brief Plane geometry data
 *
 * Planes do not have any data; they are only used
 * for representation
 */
struct PhysicsGeometryPlane {};

/**
 * @brief Capsule geometry data
 */
struct PhysicsGeometryCapsule {
  /**
   * Capsule radius
   */
  f32 radius = 1.0f;

  /**
   * Capsule half height
   */
  f32 halfHeight = 0.5f;
};

/**
 * @brief Box geometry data
 */
struct PhysicsGeometryBox {
  /**
   * Box extents halved
   */
  glm::vec3 halfExtents{0.5f};
};

using PhysicsGeometryParams =
    std::variant<quoll::PhysicsGeometryBox, quoll::PhysicsGeometrySphere,
                 quoll::PhysicsGeometryCapsule, quoll::PhysicsGeometryPlane>;

/**
 * @brief Describes geometry
 */
struct PhysicsGeometryDesc {
  /**
   * Geometry type
   */
  PhysicsGeometryType type = PhysicsGeometryType::Box;

  /**
   * Geometry center;
   */
  glm::vec3 center{0.0f};

  /**
   * Geometry parameters
   */
  PhysicsGeometryParams params = PhysicsGeometryBox{};
};

/**
 * @brief Describes dynamic rigid body
 */
struct PhysicsDynamicRigidBodyDesc {
  /**
   * Mass
   */
  f32 mass = 1.0f;

  /**
   * Inertia
   */
  glm::vec3 inertia;

  /**
   * Apply gravity
   */
  bool applyGravity = true;
};

} // namespace quoll
