// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

.pragma library

/**
 * Returns the screen aspect ratio clamped to [1.0, 3.6] to keep wizard
 * previews usable on extreme aspect ratios (e.g. 32:9 ultrawides or
 * portrait monitors).  Falls back to 16:9 if screen dimensions are
 * unavailable.
 *
 * @param {real} screenWidth  - Screen.width from the calling QML context
 * @param {real} screenHeight - Screen.height from the calling QML context
 * @returns {real}
 */
function clampedScreenAspectRatio(screenWidth, screenHeight) {
    if (screenWidth > 0 && screenHeight > 0)
        return Math.max(1, Math.min(3.6, screenWidth / screenHeight));
    return 16 / 9;
}
