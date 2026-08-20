// Copyright 2026 Mowgli Project
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <chrono>
#include <cstdint>

#include "mowgli_behavior/bt_context.hpp"

namespace mowgli_behavior
{

inline constexpr auto kRecordingTransitionGrace = std::chrono::seconds(2);

inline void completeRecordingCommand(
    BTContext& ctx,
    uint8_t expected_command,
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now())
{
  if (ctx.current_command == expected_command)
  {
    ctx.current_command = 0;
  }
  ctx.recording_transition_until = now + kRecordingTransitionGrace;
}

}  // namespace mowgli_behavior
