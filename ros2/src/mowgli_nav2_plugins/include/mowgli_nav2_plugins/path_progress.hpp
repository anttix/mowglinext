// Copyright 2026 Mowgli Project
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>
#include <cstddef>

namespace mowgli_nav2_plugins
{

inline double controllerPathProgress(std::size_t current_index, std::size_t path_size)
{
  if (path_size < 2)
  {
    return 0.0;
  }

  const std::size_t final_index = path_size - 2;
  if (final_index == 0)
  {
    return 1.0;
  }

  return static_cast<double>(std::min(current_index, final_index)) /
         static_cast<double>(final_index);
}

inline double effectivePathProgress(double inferred_progress, double controller_progress)
{
  return std::max(std::clamp(inferred_progress, 0.0, 1.0),
                  std::clamp(controller_progress, 0.0, 1.0));
}

}  // namespace mowgli_nav2_plugins
