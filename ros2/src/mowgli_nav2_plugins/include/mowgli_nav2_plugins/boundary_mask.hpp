// Copyright 2026 Mowgli Project
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

namespace mowgli_nav2_plugins
{

inline bool boundaryMaskBlocked(std::int8_t occupancy)
{
  return occupancy != 0;
}

}  // namespace mowgli_nav2_plugins
