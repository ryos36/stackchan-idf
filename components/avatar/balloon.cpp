// SPDX-FileCopyrightText: 2026 Kenta IDA <fuga@fugafuga.org>
// SPDX-License-Identifier: BSL-1.0

#include "balloon.hpp"

#include <algorithm>

namespace stackchan::avatar::internal {

namespace {

// Geometry of the bottom-of-screen balloon. Panel dimensions are computed at
// draw time from the live canvas size so the same code paths drive both the
// CoreS3 (320x240) and the AtomS3R (128x128) displays.
constexpr std::int16_t kMargin = 4;
constexpr std::int16_t kPanelRadius = 10;
constexpr std::int16_t kInnerPadding = 8;
// Small panels (e.g. AtomS3R's 128x128) get the 12-px glyph; the 24-px glyph
// would eat half the screen height and leave no room for the avatar above.
constexpr std::int16_t kSmallPanelHeightThreshold = 160;
constexpr std::int16_t kBigPanelH = 40;   // for 24-px font
constexpr std::int16_t kSmallPanelH = 22; // for 12-px font

// Marquee tuning.
constexpr std::int32_t kScrollSpeedPxPerSec = 60;

// Default minimum display time for short (non-scrolling) text. The application
// can override with `Avatar::set_balloon_text(text, hold_ms)`.
constexpr std::uint32_t kDefaultStaticHoldMs = 3000;

} // namespace

void draw_balloon(RichCanvas& canvas, DrawContext& ctx)
{
    if (!ctx.balloon_text.has_value()) {
        return;
    }
    const auto& text = *ctx.balloon_text;
    if (text.empty()) {
        return;
    }

    const std::uint16_t fg = ctx.palette.balloon_foreground;
    const std::uint16_t bg = ctx.palette.balloon_background;

    const std::int16_t canvas_w = static_cast<std::int16_t>(canvas.width());
    const std::int16_t canvas_h = static_cast<std::int16_t>(canvas.height());
    const bool small_panel = canvas_h <= kSmallPanelHeightThreshold;
    const auto* font = small_panel ? &fonts::lgfxJapanGothic_12 : &fonts::lgfxJapanGothic_24;
    const std::int16_t panel_h = small_panel ? kSmallPanelH : kBigPanelH;
    const std::int16_t panel_x = kMargin;
    const std::int16_t panel_w = canvas_w - kMargin * 2;
    const std::int16_t panel_y = canvas_h - panel_h - kMargin;

    // Composite the bottom balloon strip as one group (direct strategy clears +
    // blits the whole panel; buffered strategy treats it as a no-op).
    canvas.begin_group(panel_x, panel_y, panel_w, panel_h);
    canvas.fillRoundRect(panel_x, panel_y, panel_w, panel_h, kPanelRadius, bg);
    canvas.drawRoundRect(panel_x, panel_y, panel_w, panel_h, kPanelRadius, fg);

    canvas.setTextColor(fg, bg);
    canvas.setFont(font);
    canvas.setTextSize(1);

    const std::int32_t inner_x = panel_x + kInnerPadding;
    const std::int32_t inner_w = panel_w - 2 * kInnerPadding;
    const std::int32_t text_w = canvas.textWidth(text.c_str());
    const std::int32_t mid_y = panel_y + panel_h / 2;
    const std::uint32_t elapsed_ms = ctx.now_ms - ctx.balloon_set_ms;

    if (!ctx.balloon_ever_streamed && text_w <= inner_w) {
        // Text fits — static centered. Mark done after the configured hold.
        // Gated on balloon_ever_streamed (sticky), not just balloon_streaming
        // (instantaneous): a reply that streamed in must keep using the
        // marquee's position formula below for its entire life, even after
        // streaming ends and the final text turns out to be short. Switching
        // rendering modes at that point — to this centered layout — would
        // still jump, just one frame later than switching mid-stream would.
        canvas.setTextDatum(lgfx::textdatum_t::middle_center);
        canvas.drawString(text.c_str(), panel_x + panel_w / 2, mid_y);

        const std::uint32_t hold_ms =
            std::max(ctx.balloon_hold_ms, kDefaultStaticHoldMs);
        if (elapsed_ms >= hold_ms) {
            ctx.balloon_done = true;
        }
        canvas.end_group();
        return;
    }

    // Marquee: text starts just past the right inner edge and scrolls left
    // until it reaches a resting point, then holds there before the balloon
    // completes — it never disappears mid-motion:
    //  - long text (text_w > inner_w): rests once its tail has fully
    //    entered (left edge at inner_x + inner_w - text_w), leaving the
    //    trailing portion flush against the right edge.
    //  - short text (text_w <= inner_w — only reachable here once a balloon
    //    has streamed; see balloon_ever_streamed above): resting flush
    //    right after barely moving read as an odd, half-finished slide, so
    //    it keeps going until flush against the LEFT edge instead.
    // Both are "whichever needs more travel", so one formula covers both,
    // and while streaming (text_w still growing) the transition between
    // them stays continuous — the cap only ever loosens as text_w grows.
    const std::int32_t travel_px = std::max(text_w, inner_w);
    const std::int32_t offset =
        std::min(static_cast<std::int32_t>(elapsed_ms) * kScrollSpeedPxPerSec / 1000, travel_px);
    const std::int32_t x = inner_x + inner_w - offset;

    canvas.setClipRect(inner_x, panel_y, inner_w, panel_h);
    canvas.setTextDatum(lgfx::textdatum_t::middle_left);
    canvas.drawString(text.c_str(), x, mid_y);
    canvas.clearClipRect();

    // Mark done once the resting point above has been held for the hold
    // duration (or the caller-requested hold time, whichever is longer).
    const std::uint32_t travel_ms =
        static_cast<std::uint32_t>(travel_px) * 1000u / static_cast<std::uint32_t>(kScrollSpeedPxPerSec);
    const std::uint32_t hold_ms = std::max(ctx.balloon_hold_ms, kDefaultStaticHoldMs);
    const std::uint32_t complete_at = travel_ms + hold_ms;
    if (!ctx.balloon_streaming && elapsed_ms >= complete_at) {
        ctx.balloon_done = true;
    }
    canvas.end_group();
}

} // namespace stackchan::avatar::internal
