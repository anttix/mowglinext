// Copyright 2026 Mowgli Project
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cmath>

#include "mowgli_nav2_plugins/controller_math.hpp"
#include <gtest/gtest.h>

namespace mowgli_nav2_plugins
{
namespace
{

TEST(ControllerMath, WrapsAccumulatedHeadingForControl)
{
  constexpr double kDegreesToRadians = M_PI / 180.0;

  EXPECT_NEAR(wrappedControllerAngle(345.0 * kDegreesToRadians), -15.0 * kDegreesToRadians, 1e-12);
  EXPECT_NEAR(wrappedControllerAngle(554.0 * kDegreesToRadians), -166.0 * kDegreesToRadians, 1e-12);
}

TEST(ControllerMath, StartsClosedPathAtFrontWhenRobotIsAlreadyThere)
{
  EXPECT_EQ(selectInitialPathIndex(0.08, 328, 1.0), 0U);
}

TEST(ControllerMath, ResynchronizesWhenRobotIsFarFromPathFront)
{
  EXPECT_EQ(selectInitialPathIndex(1.01, 328, 1.0), 328U);
}

TEST(ControllerMath, BoundsResynchronizationToAForwardWindow)
{
  EXPECT_EQ(boundedResyncEnd(420, 9277, 50), 471U);
  EXPECT_EQ(boundedResyncEnd(9250, 9277, 50), 9277U);
  EXPECT_EQ(boundedResyncEnd(9277, 9277, 50), 9277U);
}

}  // namespace
}  // namespace mowgli_nav2_plugins
