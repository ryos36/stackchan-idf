// SPDX-FileCopyrightText: 2026 Kenta IDA <fuga@fugafuga.org>
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "avatar/expression.hpp"
#include "avatar/palette.hpp"

namespace stackchan::avatar {

struct DrawContext {
    Expression expression{Expression::Neutral};
    float breath{0.0f};
    // External / commanded gaze target — written by Avatar::set_gaze (e.g.
    // touch-follow, future API gesture). Saccade is added on top so the
    // eyes still wander while pointing roughly at the target.
    float gaze_horizontal{0.0f};
    float gaze_vertical{0.0f};
    // Animator-driven saccade offset, summed with the target above by
    // Var::GazeH / Var::GazeV in the VM. Updated by FaceAnimator::
    // saccade_tick; kept zero when saccade is disabled.
    float gaze_saccade_h{0.0f};
    float gaze_saccade_v{0.0f};
    float eye_open_ratio{1.0f};
    float mouth_open_ratio{0.0f};
    Palette palette{kDefaultPalette};
    std::uint32_t rng_state{0xC0FFEEu};
    std::optional<std::string> balloon_text{};
    // Wall-clock used for time-based animation (e.g. balloon marquee).
    std::uint32_t now_ms{0};
    // Set to `now_ms` whenever balloon_text changes — drives marquee phase.
    std::uint32_t balloon_set_ms{0};
    // Minimum display time. 0 means "use balloon defaults" (short = a fixed
    // hold, long = one marquee pass).
    std::uint32_t balloon_hold_ms{0};
    // Set by balloon rendering once the message has been displayed in full
    // (i.e. hold time elapsed for short text, or one marquee cycle for long
    // text). The render task polls this and notifies the application.
    bool balloon_done{false};
    // True while more text is still expected (a reply streaming in).
    // Suppresses the balloon_done check below so the balloon doesn't
    // auto-complete (and get hidden by the render task) mid-stream —
    // scrolling still runs normally against balloon_set_ms throughout.
    // Cleared once the caller knows the text is final.
    bool balloon_streaming{false};
    // Sticky version of balloon_streaming: set the moment streaming starts,
    // never cleared back to false until a fresh (non-streaming) balloon
    // replaces this one. Unlike balloon_streaming, this stays true after
    // the reply finishes — it gates which rendering mode balloon.cpp uses
    // (see there), so a balloon that streamed in never switches to the
    // static-centered layout even once its final text turns out to be
    // short: that switch would jump instead of using the same continuous
    // position function throughout.
    bool balloon_ever_streamed{false};
};

} // namespace stackchan::avatar
