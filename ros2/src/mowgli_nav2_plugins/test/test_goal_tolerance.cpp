// Copyright 2026 Mowgli Project
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cmath>

#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include "mowgli_nav2_plugins/goal_tolerance.hpp"
#include <gtest/gtest.h>

namespace mowgli_nav2_plugins
{
namespace
{

geometry_msgs::msg::Pose makePose(double x, double y, double yaw)
{
  geometry_msgs::msg::Pose pose;
  pose.position.x = x;
  pose.position.y = y;
  tf2::Quaternion orientation;
  orientation.setRPY(0.0, 0.0, yaw);
  pose.orientation = tf2::toMsg(orientation);
  return pose;
}

TEST(GoalTolerance, AcceptsConfiguredPositionBoundary)
{
  const GoalPoseError error = goalPoseError(makePose(0.3, 0.4, 0.0), makePose(0.0, 0.0, 0.0));

  EXPECT_DOUBLE_EQ(error.xy, 0.5);
  EXPECT_TRUE(withinGoalTolerance(error, 0.5, 0.1));
  EXPECT_FALSE(withinGoalTolerance(error, 0.499, 0.1));
}

TEST(GoalTolerance, WrapsYawAcrossPi)
{
  constexpr double kDegreesToRadians = M_PI / 180.0;
  const GoalPoseError error = goalPoseError(makePose(0.0, 0.0, 179.0 * kDegreesToRadians),
                                            makePose(0.0, 0.0, -179.0 * kDegreesToRadians));

  EXPECT_NEAR(std::abs(error.yaw), 2.0 * kDegreesToRadians, 1e-12);
  EXPECT_TRUE(withinGoalTolerance(error, 0.5, 3.0 * kDegreesToRadians));
  EXPECT_FALSE(withinGoalTolerance(error, 0.5, 1.0 * kDegreesToRadians));
}

}  // namespace
}  // namespace mowgli_nav2_plugins
