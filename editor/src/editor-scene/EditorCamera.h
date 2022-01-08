#pragma once

#include "scene/Camera.h"
#include "renderer/vulkan/VulkanRenderer.h"
#include "window/glfw/GLFWWindow.h"
#include "entity/EntityContext.h"

namespace liquidator {

class EditorCamera {
public:
  enum class InputState { None = 0, Pan = 1, Rotate = 2, Zoom = 3 };

  static constexpr float DEFAULT_FOV = 70.0f;
  static constexpr float DEFAULT_NEAR = 0.001f;
  static constexpr float DEFAULT_FAR = 1000.0f;
  static constexpr glm::vec3 DEFAULT_EYE{0.0f, 5.0f, -10.0f};
  static constexpr glm::vec3 DEFAULT_CENTER{0.0f, 0.0f, 0.0f};
  static constexpr glm::vec3 DEFAULT_UP{0.0f, 1.0f, 0.0f};

public:
  /**
   * @brief Create editor camera
   *
   * @param context Entity context
   * @param renderer Renderer
   * @param window Window
   */
  EditorCamera(liquid::EntityContext &context, liquid::VulkanRenderer *renderer,
               liquid::GLFWWindow *window);

  EditorCamera(const EditorCamera &) = delete;
  EditorCamera &operator=(const EditorCamera &) = delete;
  EditorCamera(EditorCamera &&) = delete;
  EditorCamera &operator=(EditorCamera &&) = delete;

  /**
   * @brief Destroy editor camera
   *
   * Cleans up all handlers
   */
  ~EditorCamera();

  /**
   * @brief Set field of view
   *
   * @param fov Field of view
   */
  inline void setFOV(float fov_) { fov = fov_; }

  /**
   * @brief Set near plane
   *
   * @param near Near plane
   */
  inline void setNear(float near_) { near = near_; }

  /**
   * @brief Set far plane
   *
   * @param Far plane
   */
  inline void setFar(float far_) { far = far_; }

  /**
   * @brief Get field of view
   *
   * @return Field of view
   */
  inline float getFOV() { return fov; }

  /**
   * @brief Get near place
   *
   * @return Near plane
   */
  inline float getNear() { return near; }

  /**
   * @brief Get far plane
   *
   * @return Far plane
   */
  inline float getFar() { return far; }

  /**
   * @brief Get camera
   *
   * @return Camera
   */
  inline const liquid::Entity getCamera() const { return cameraEntity; }

  /**
   * @brief Set camera center
   *
   * @param center Center vector
   */
  void setCenter(const glm::vec3 &center);

  /**
   * @brief Set camera eye position
   *
   * @param eye Eye vector
   */
  void setEye(const glm::vec3 &eye);

  /**
   * @brief Update camera
   */
  void update();

  /**
   * @brief Reset camera to defaults
   */
  void reset();

  /**
   * @brief Set viewport
   *
   * @param x Viewport x position
   * @param y Viewport y position
   * @param width Viewport width
   * @param height Viewport height
   */
  void setViewport(float x, float y, float width, float height);

  /**
   * @brief Get input state
   *
   * @return Input state
   */
  inline const InputState &getInputState() const { return inputState; }

private:
  /**
   * @brief Pan camera using mouse movement
   */
  void pan();

  /**
   * @brief Rotate camera using mouse movement
   */
  void rotate();

  /**
   * @brief Zoom camera using mouse movement
   */
  void zoom();

  /**
   * @brief Update perspective
   *
   * @param aspectRatio Aspect ratio
   */
  void updatePerspective(float aspectRatio);

private:
  float fov = DEFAULT_FOV;
  float near = DEFAULT_NEAR;
  float far = DEFAULT_FAR;

  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;

  InputState inputState = InputState::None;
  glm::vec2 prevMousePos{};

  glm::vec3 eye = DEFAULT_EYE;
  glm::vec3 center = DEFAULT_CENTER;
  glm::vec3 up = DEFAULT_UP;

  uint32_t mouseButtonHandler;
  uint32_t mouseMoveHandler;

  liquid::GLFWWindow *window;
  liquid::SharedPtr<liquid::Camera> camera;
  liquid::EntityContext &context;
  liquid::Entity cameraEntity = liquid::ENTITY_MAX;
};

} // namespace liquidator
