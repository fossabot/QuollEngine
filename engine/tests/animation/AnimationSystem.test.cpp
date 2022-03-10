#include "liquid/core/Base.h"
#include "liquid/animation/AnimationSystem.h"
#include "liquid/scene/Skeleton.h"

#include <gtest/gtest.h>

class AnimationSystemTest : public ::testing::Test {
public:
  liquid::EntityContext context;
  liquid::AnimationSystem system;
  liquid::experimental::ResourceRegistry registry;

  AnimationSystemTest() : system(context) {}

  liquid::Entity createEntity(bool loop, uint32_t animIndex = 0,
                              bool playing = true) {
    auto entity = context.createEntity();
    context.setComponent<liquid::TransformComponent>(entity, {});
    context.setComponent<liquid::AnimatorComponent>(
        entity, {0, loop, 0.0f, playing, {animIndex}});

    return entity;
  }

  liquid::Entity createEntityWithSkeleton(bool loop, uint32_t animIndex = 0,
                                          bool playing = true) {
    auto entity = createEntity(loop, animIndex, playing);

    liquid::Skeleton skeleton(
        {glm::vec3{0.0f}}, {glm::quat{1.0f, 0.0f, 0.0f, 0.0f}},
        {glm::vec3{1.0f}}, {0}, {glm::mat4{1.0f}}, {"Joint0"}, &registry);

    context.setComponent<liquid::SkeletonComponent>(entity, {skeleton});

    return entity;
  }

  uint32_t createAnimation(liquid::KeyframeSequenceTarget target, float time) {
    liquid::Animation animation("testAnim", time);
    liquid::KeyframeSequence sequence(
        target, liquid::KeyframeSequenceInterpolation::Step);

    sequence.addKeyframe(0.0f, glm::vec4(0.0f));
    sequence.addKeyframe(0.5f, glm::vec4(0.5f));
    sequence.addKeyframe(1.0f, glm::vec4(1.0f));

    animation.addKeyframeSequence(sequence);
    return system.addAnimation(animation);
  }

  uint32_t createSkeletonAnimation(liquid::KeyframeSequenceTarget target,
                                   float time) {
    liquid::Animation animation("testAnim", time);
    liquid::KeyframeSequence sequence(
        target, liquid::KeyframeSequenceInterpolation::Step, 0);

    sequence.addKeyframe(0.0f, glm::vec4(0.0f));
    sequence.addKeyframe(0.5f, glm::vec4(0.5f));
    sequence.addKeyframe(1.0f, glm::vec4(1.0f));

    animation.addKeyframeSequence(sequence);
    return system.addAnimation(animation);
  }
};

using AnimationSystemDeathTest = AnimationSystemTest;

TEST_F(AnimationSystemTest, AddsAnimation) {
  uint32_t index =
      createAnimation(liquid::KeyframeSequenceTarget::Position, 2.0f);

  EXPECT_EQ(system.getAnimation(index).getName(), "testAnim");
  EXPECT_EQ(system.getAnimation(index).getTime(), 2.0f);
}

TEST_F(AnimationSystemDeathTest, FailsIfNonExistentAnimationIsRequested) {
  EXPECT_DEATH(system.getAnimation(12), ".*");
}

TEST_F(AnimationSystemTest,
       DoesNotAdvanceEntityAnimationIfAnimationDoesNotExist) {
  auto entity = createEntity(false, 1);
  const auto &animation =
      context.getComponent<liquid::AnimatorComponent>(entity);

  EXPECT_EQ(animation.normalizedTime, 0.0f);
  system.update(0.5f);
  EXPECT_EQ(animation.normalizedTime, 0.0f);
}

TEST_F(AnimationSystemTest, DoesNotAdvanceTimeIfComponentIsNotPlaying) {
  auto animIndex =
      createAnimation(liquid::KeyframeSequenceTarget::Position, 2.0f);
  auto entity = createEntity(false, animIndex, false);

  const auto &animation =
      context.getComponent<liquid::AnimatorComponent>(entity);
  EXPECT_EQ(animation.normalizedTime, 0.0f);
  system.update(0.5f);
  EXPECT_EQ(animation.normalizedTime, 0.0f);
}

TEST_F(AnimationSystemTest,
       AdvancedEntityAnimationNormalizedTimeByDeltaTimeAndAnimationSpeed) {
  createAnimation(liquid::KeyframeSequenceTarget::Position, 2.0f);
  auto entity = createEntity(false);

  const auto &animation =
      context.getComponent<liquid::AnimatorComponent>(entity);
  EXPECT_EQ(animation.normalizedTime, 0.0f);
  system.update(0.5f);
  EXPECT_EQ(animation.normalizedTime, 0.25f);
}

TEST_F(AnimationSystemTest, PausesEntityAnimationWhenItReachesAnimationEnd) {
  createAnimation(liquid::KeyframeSequenceTarget::Position, 1.0f);
  auto entity = createEntity(false);

  const auto &animation =
      context.getComponent<liquid::AnimatorComponent>(entity);
  EXPECT_EQ(animation.normalizedTime, 0.0f);
  system.update(1.5f);
  EXPECT_EQ(animation.normalizedTime, 1.0f);
  system.update(1.5f);
  EXPECT_EQ(animation.normalizedTime, 1.0f);
}

TEST_F(AnimationSystemTest, RestartsAnimationTimeIfLoop) {
  createAnimation(liquid::KeyframeSequenceTarget::Position, 1.0f);
  auto entity = createEntity(true);
  const auto &animation =
      context.getComponent<liquid::AnimatorComponent>(entity);
  EXPECT_EQ(animation.normalizedTime, 0.0f);
  system.update(1.0f);
  EXPECT_EQ(animation.normalizedTime, 0.0f);
}

TEST_F(AnimationSystemTest, UpdateEntityPositionBasedOnPositionKeyframe) {
  createAnimation(liquid::KeyframeSequenceTarget::Position, 1.0f);
  auto entity = createEntity(false);

  const auto &transform =
      context.getComponent<liquid::TransformComponent>(entity);

  EXPECT_EQ(transform.localPosition, glm::vec3(0.0f));
  EXPECT_EQ(transform.localRotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(transform.localScale, glm::vec3(1.0f));
  system.update(0.5f);

  EXPECT_EQ(transform.localPosition, glm::vec3(0.5f));
  EXPECT_EQ(transform.localRotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(transform.localScale, glm::vec3(1.0f));
}

TEST_F(AnimationSystemTest, UpdateEntityRotationBasedOnRotationKeyframe) {
  createAnimation(liquid::KeyframeSequenceTarget::Rotation, 1.0f);
  auto entity = createEntity(false);

  const auto &transform =
      context.getComponent<liquid::TransformComponent>(entity);

  EXPECT_EQ(transform.localPosition, glm::vec3(0.0f));
  EXPECT_EQ(transform.localRotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(transform.localScale, glm::vec3(1.0f));
  system.update(0.5f);

  EXPECT_EQ(transform.localPosition, glm::vec3(0.0f));
  EXPECT_EQ(transform.localRotation, glm::quat(0.5f, 0.5f, 0.5f, 0.5f));
  EXPECT_EQ(transform.localScale, glm::vec3(1.0f));
}

TEST_F(AnimationSystemTest, UpdateEntityScaleBasedOnScaleKeyframe) {
  createAnimation(liquid::KeyframeSequenceTarget::Scale, 1.0f);
  auto entity = createEntity(false);

  const auto &transform =
      context.getComponent<liquid::TransformComponent>(entity);

  EXPECT_EQ(transform.localPosition, glm::vec3(0.0f));
  EXPECT_EQ(transform.localRotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(transform.localScale, glm::vec3(1.0f));
  system.update(0.5f);

  EXPECT_EQ(transform.localPosition, glm::vec3(0.0f));
  EXPECT_EQ(transform.localRotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(transform.localScale, glm::vec3(0.5f));
}

TEST_F(AnimationSystemTest,
       UpdateSkeletonPositionBasedOnPositionKeyframeWithJointTarget) {
  createSkeletonAnimation(liquid::KeyframeSequenceTarget::Position, 1.0f);
  auto entity = createEntityWithSkeleton(false);

  const auto &transform =
      context.getComponent<liquid::TransformComponent>(entity);

  const auto &skeleton =
      context.getComponent<liquid::SkeletonComponent>(entity);
  EXPECT_EQ(skeleton.skeleton.getLocalPosition(0), glm::vec3(0.0f));
  EXPECT_EQ(skeleton.skeleton.getLocalRotation(0),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(skeleton.skeleton.getLocalScale(0), glm::vec3(1.0f));
  system.update(0.5f);
  EXPECT_EQ(skeleton.skeleton.getLocalPosition(0), glm::vec3(0.5f));
  EXPECT_EQ(skeleton.skeleton.getLocalRotation(0),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(skeleton.skeleton.getLocalScale(0), glm::vec3(1.0f));

  EXPECT_EQ(transform.localPosition, glm::vec3(0.0f));
  EXPECT_EQ(transform.localRotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(transform.localScale, glm::vec3(1.0f));
}

TEST_F(AnimationSystemTest,
       UpdateSkeletonRotationBasedOnRotationKeyframeWithJointTarget) {
  createSkeletonAnimation(liquid::KeyframeSequenceTarget::Rotation, 1.0f);
  auto entity = createEntityWithSkeleton(false);

  const auto &transform =
      context.getComponent<liquid::TransformComponent>(entity);

  const auto &skeleton =
      context.getComponent<liquid::SkeletonComponent>(entity);
  EXPECT_EQ(skeleton.skeleton.getLocalPosition(0), glm::vec3(0.0f));
  EXPECT_EQ(skeleton.skeleton.getLocalRotation(0),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(skeleton.skeleton.getLocalScale(0), glm::vec3(1.0f));
  system.update(0.5f);
  EXPECT_EQ(skeleton.skeleton.getLocalPosition(0), glm::vec3(0.0f));
  EXPECT_EQ(skeleton.skeleton.getLocalRotation(0),
            glm::quat(0.5f, 0.5f, 0.5f, 0.5f));
  EXPECT_EQ(skeleton.skeleton.getLocalScale(0), glm::vec3(1.0f));

  EXPECT_EQ(transform.localPosition, glm::vec3(0.0f));
  EXPECT_EQ(transform.localRotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(transform.localScale, glm::vec3(1.0f));
}

TEST_F(AnimationSystemTest,
       UpdateSkeletonScaleBasedOnScaleKeyframeWithJointTarget) {
  createSkeletonAnimation(liquid::KeyframeSequenceTarget::Scale, 1.0f);
  auto entity = createEntityWithSkeleton(false);

  const auto &transform =
      context.getComponent<liquid::TransformComponent>(entity);

  const auto &skeleton =
      context.getComponent<liquid::SkeletonComponent>(entity);
  EXPECT_EQ(skeleton.skeleton.getLocalPosition(0), glm::vec3(0.0f));
  EXPECT_EQ(skeleton.skeleton.getLocalRotation(0),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(skeleton.skeleton.getLocalScale(0), glm::vec3(1.0f));
  system.update(0.5f);
  EXPECT_EQ(skeleton.skeleton.getLocalPosition(0), glm::vec3(0.0f));
  EXPECT_EQ(skeleton.skeleton.getLocalRotation(0),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(skeleton.skeleton.getLocalScale(0), glm::vec3(0.5f));

  EXPECT_EQ(transform.localPosition, glm::vec3(0.0f));
  EXPECT_EQ(transform.localRotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  EXPECT_EQ(transform.localScale, glm::vec3(1.0f));
}
