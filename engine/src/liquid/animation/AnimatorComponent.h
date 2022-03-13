#pragma once

namespace liquid {

struct AnimatorComponent {
  uint32_t currentAnimation = 0;
  bool loop = false;
  float normalizedTime = 0.0f;
  bool playing = false;

  std::vector<uint32_t> animations;
};

} // namespace liquid