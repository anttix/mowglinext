// Copyright 2026 Mowgli Project
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mowgli_nav2_plugins/coverage_completion.hpp"
#include <gtest/gtest.h>

namespace mowgli_nav2_plugins
{
namespace
{

TEST(CoverageCompletion, RequiresNominalPathAfterAvoidance)
{
  EXPECT_TRUE(deviationSettledForCompletion(false, false, 0.0));
  EXPECT_TRUE(deviationSettledForCompletion(false, false, 0.009));
  EXPECT_FALSE(deviationSettledForCompletion(true, false, 0.0));
  EXPECT_FALSE(deviationSettledForCompletion(false, true, 0.0));
  EXPECT_FALSE(deviationSettledForCompletion(false, false, 0.01));
  EXPECT_FALSE(deviationSettledForCompletion(false, false, -0.25));
}

}  // namespace
}  // namespace mowgli_nav2_plugins
