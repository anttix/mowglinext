// Copyright 2026 Mowgli Project
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cmath>

namespace mowgli_nav2_plugins
{

inline bool deviationSettledForCompletion(bool is_avoiding,
                                          bool obstacle_waiting,
                                          double lateral_deviation)
{
  return !is_avoiding && !obstacle_waiting && std::abs(lateral_deviation) < 0.01;
}

}  // namespace mowgli_nav2_plugins
