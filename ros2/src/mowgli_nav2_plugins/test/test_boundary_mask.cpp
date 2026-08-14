// Copyright 2026 Mowgli Project
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mowgli_nav2_plugins/boundary_mask.hpp"
#include <gtest/gtest.h>

namespace mowgli_nav2_plugins
{
namespace
{

TEST(BoundaryMask, AllowsOnlyMowingAreaCells)
{
  EXPECT_FALSE(boundaryMaskBlocked(0));
  EXPECT_TRUE(boundaryMaskBlocked(1));
  EXPECT_TRUE(boundaryMaskBlocked(50));
  EXPECT_TRUE(boundaryMaskBlocked(100));
  EXPECT_TRUE(boundaryMaskBlocked(-1));
}

}  // namespace
}  // namespace mowgli_nav2_plugins
