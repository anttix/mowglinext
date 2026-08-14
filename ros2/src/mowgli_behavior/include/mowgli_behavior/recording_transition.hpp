// Copyright 2026 Mowgli Project
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
