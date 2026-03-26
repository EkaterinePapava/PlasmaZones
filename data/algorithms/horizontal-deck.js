// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

// @name Horizontal Deck
// @description Focused window on top; remaining windows peek from the bottom edge
// @producesOverlappingZones true
// @supportsMasterCount false
// @supportsSplitRatio true
// @defaultSplitRatio 0.75
// @defaultMaxWindows 5
// @minimumWindows 1
// @zoneNumberDisplay firstAndLast

/**
 * Horizontal Deck layout: vertical version of Deck. The focused window takes
 * the top portion, remaining windows peek from the bottom edge.
 * splitRatio controls the focused window height as a fraction of the screen.
 *
 * Each background window extends from its peek position all the way to the
 * bottom edge, so they overlap each other.
 *
 * @param {Object} params - Tiling parameters
 * @returns {Array<{x: number, y: number, width: number, height: number}>}
 */
function calculateZones(params) {
    var count = params.windowCount;
    var area = params.area;

    if (count <= 0) return [];
    if (count === 1) return [area];

    var focusedFraction = params.splitRatio > 0 ? params.splitRatio : 0.75;
    var bgCount = count - 1;
    var focusedHeight = Math.max(1, Math.round(area.height * focusedFraction));
    var peekTotal = area.height - focusedHeight;
    var peekHeight = bgCount > 0 ? Math.max(1, Math.round(peekTotal / bgCount)) : 0;

    var zones = [];

    // First zone: the focused (top) window
    zones.push({
        x: area.x,
        y: area.y,
        width: area.width,
        height: focusedHeight
    });

    // Background windows: each starts at its peek position and
    // extends to the bottom edge of the area
    for (var i = 0; i < bgCount; i++) {
        var peekY = area.y + focusedHeight + (i * peekHeight);
        zones.push({
            x: area.x,
            y: peekY,
            width: area.width,
            height: area.y + area.height - peekY
        });
    }

    return zones;
}
