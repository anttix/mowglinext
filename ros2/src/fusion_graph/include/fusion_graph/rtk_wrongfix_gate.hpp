// Copyright 2026 Mowgli Project
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pure RTK wrong-fix motion-consistency gate, factored out of OnGnss so it is
// unit-testable without ROS/GTSAM (see fusion_graph_node_callbacks_a.cpp).
// F9P can re-solve the carrier-phase ambiguity on a different integer set
// after a brief signal drop (vegetation, multipath spike) and the new
// solution jumps by a few cm while still reporting a trustworthy status +
// sub-cm covariance. The gate rejects a GPS step that is larger than the
// chassis could plausibly have travelled since the last fix (wheel arc +
// lever-arm sweep during rotation) plus a fixed slack budget.
//
// CRITICAL DESIGN NOTE — bounded vs. runaway: the two accumulators
// (wheel_dist_m, abs_dtheta_rad) MUST be reset after every distinct fix,
// whether it is ACCEPTED or REJECTED. A small bounded exception lets the node
// retain motion across at most three repeated-coordinate epochs: some receivers
// and simulators republish the previous position before a delayed update, so
// the next distinct step spans multiple reporting intervals. The reverted
// GnssMobileGate
// (bundled into PR #307) instead reset its equivalent accumulator only on
// ACCEPT: once it started rejecting, the accumulator never cleared, motion
// kept piling up against a stale reference, "expected motion" ran away
// (observed 20.9 m for <1 m of real travel), and every subsequent fix
// rejected forever (GPS locked out, cov_xx ballooned to ~2.5 m σ). See
// CLAUDE.md "What NOT to Do" for the incident writeup.

#pragma once

#include <cmath>
#include <optional>

namespace fusion_graph
{

inline std::optional<double> UsableGnssSigma(double var_x,
                                             double var_y,
                                             bool covariance_known,
                                             double speed_mps,
                                             double speed_coeff_s,
                                             double max_sigma_m)
{
  double sigma = std::sqrt(0.5 * (var_x + var_y));
  if (!covariance_known || !std::isfinite(sigma) || sigma <= 0.0)
  {
    return std::nullopt;
  }
  if (speed_coeff_s > 0.0)
  {
    const double speed_term = speed_coeff_s * std::abs(speed_mps);
    sigma = std::sqrt(sigma * sigma + speed_term * speed_term);
  }
  if (max_sigma_m > 0.0 && sigma > max_sigma_m)
  {
    return std::nullopt;
  }
  return sigma;
}

// True if a GPS step of `jump_m` cannot be explained by chassis motion since
// the last fix (wheel-arc distance `wheel_dist_m` plus lever-arm sweep
// `lever_arm_radius_m * abs_dtheta_rad` from in-place rotation) plus the fixed
// slack budget `max_jump_m` — i.e. the fix should be dropped as a likely
// wrong-fix rather than fused.
inline bool GpsJumpImplausible(double jump_m,
                               double max_jump_m,
                               double lever_arm_radius_m,
                               double abs_dtheta_rad,
                               double wheel_dist_m)
{
  const double expected_pivot_jump_m = lever_arm_radius_m * abs_dtheta_rad;
  const double jump_budget_m = max_jump_m + expected_pivot_jump_m + wheel_dist_m;
  return jump_m > jump_budget_m;
}

inline bool HoldMotionBudgetForRepeatedFix(double jump_m,
                                           unsigned int held_epochs,
                                           unsigned int max_held_epochs = 3,
                                           double repeated_fix_epsilon_m = 1e-6)
{
  return jump_m <= repeated_fix_epsilon_m && held_epochs < max_held_epochs;
}

inline bool WrongFixCandidateConfirmed(double candidate_step_m,
                                       double max_jump_m,
                                       double lever_arm_radius_m,
                                       double abs_dtheta_rad,
                                       double wheel_dist_m)
{
  return !GpsJumpImplausible(
      candidate_step_m, max_jump_m, lever_arm_radius_m, abs_dtheta_rad, wheel_dist_m);
}

inline bool WrongFixCandidateReady(unsigned int agreeing_confirmations,
                                   unsigned int required_confirmations = 3)
{
  return agreeing_confirmations >= required_confirmations;
}

// Unconditional post-fix reset of the bounded motion accumulators. Call this
// after every DISTINCT GPS fix regardless of GpsJumpImplausible's verdict —
// accepted or rejected — and after the bounded repeated-fix hold is exhausted.
// The next distinct fix's budget therefore reflects at most four reporting
// intervals, never an unbounded "since last accepted fix" total. Skipping this
// call on the reject path is exactly the GnssMobileGate regression described
// above.
inline void ResetRtkWrongFixAccumulators(double& wheel_dist_m, double& abs_dtheta_rad)
{
  wheel_dist_m = 0.0;
  abs_dtheta_rad = 0.0;
}

}  // namespace fusion_graph
