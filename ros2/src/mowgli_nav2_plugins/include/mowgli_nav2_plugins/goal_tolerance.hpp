// Copyright 2026 Mowgli Project
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOWGLI_NAV2_PLUGINS__GOAL_TOLERANCE_HPP_
#define MOWGLI_NAV2_PLUGINS__GOAL_TOLERANCE_HPP_

#include <cmath>

#include <geometry_msgs/msg/pose.hpp>
#include <tf2/utils.hpp>

namespace mowgli_nav2_plugins
{

struct GoalPoseError
{
  double xy;
  double yaw;
};

inline GoalPoseError goalPoseError(const geometry_msgs::msg::Pose& query_pose,
                                   const geometry_msgs::msg::Pose& goal_pose)
{
  const double dx = query_pose.position.x - goal_pose.position.x;
  const double dy = query_pose.position.y - goal_pose.position.y;
  const double yaw_delta = tf2::getYaw(query_pose.orientation) - tf2::getYaw(goal_pose.orientation);
  return {std::hypot(dx, dy), std::atan2(std::sin(yaw_delta), std::cos(yaw_delta))};
}

inline bool withinGoalTolerance(const GoalPoseError& error,
                                double xy_tolerance,
                                double yaw_tolerance)
{
  return error.xy <= xy_tolerance && std::abs(error.yaw) <= yaw_tolerance;
}

}  // namespace mowgli_nav2_plugins

#endif  // MOWGLI_NAV2_PLUGINS__GOAL_TOLERANCE_HPP_
