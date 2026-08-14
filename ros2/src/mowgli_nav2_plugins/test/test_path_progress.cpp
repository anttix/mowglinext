// Copyright 2026 Mowgli Project
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mowgli_nav2_plugins/path_progress.hpp"
#include <gtest/gtest.h>

namespace mowgli_nav2_plugins
{
namespace
{

TEST(PathProgress, ReportsControllerTraversalAgainstUnduplicatedGoalIndex)
{
  EXPECT_GT(controllerPathProgress(328, 331), 0.95);
  EXPECT_DOUBLE_EQ(controllerPathProgress(329, 331), 1.0);
}

TEST(PathProgress, UsesTheControllerPublisherNamespace)
{
  EXPECT_EQ(controllerPathProgressTopic("FollowCoveragePath"), "/FollowCoveragePath/path_progress");
}

TEST(PathProgress, ClampsControllerIndexAtCompletion)
{
  EXPECT_DOUBLE_EQ(controllerPathProgress(500, 331), 1.0);
}

TEST(PathProgress, HandlesEmptyAndMinimalPaths)
{
  EXPECT_DOUBLE_EQ(controllerPathProgress(0, 0), 0.0);
  EXPECT_DOUBLE_EQ(controllerPathProgress(0, 1), 0.0);
  EXPECT_DOUBLE_EQ(controllerPathProgress(0, 2), 1.0);
}

TEST(PathProgress, UsesMostAdvancedAvailableEstimate)
{
  EXPECT_DOUBLE_EQ(effectivePathProgress(0.40, 0.96), 0.96);
  EXPECT_DOUBLE_EQ(effectivePathProgress(0.97, 0.50), 0.97);
  EXPECT_DOUBLE_EQ(effectivePathProgress(-1.0, 2.0), 1.0);
}

}  // namespace
}  // namespace mowgli_nav2_plugins
