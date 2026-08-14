// Copyright 2026 Mowgli Project
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cmath>
#include <cstddef>

namespace mowgli_nav2_plugins
{

inline double wrappedControllerAngle(double angle)
{
  return std::atan2(std::sin(angle), std::cos(angle));
}

inline std::size_t selectInitialPathIndex(double front_distance,
                                          std::size_t nearest_index,
                                          double start_distance_limit)
{
  return front_distance <= start_distance_limit ? 0 : nearest_index;
}

inline std::size_t boundedResyncEnd(std::size_t current_index,
                                    std::size_t path_size,
                                    std::size_t max_index_advance)
{
  const std::size_t remaining = path_size > current_index ? path_size - current_index : 0;
  return current_index + std::min(remaining, max_index_advance + 1);
}

inline double laterallyShiftedCoordinate(double nominal_coordinate,
                                         double lateral_axis_component,
                                         double deviation)
{
  return nominal_coordinate + lateral_axis_component * deviation;
}

}  // namespace mowgli_nav2_plugins
