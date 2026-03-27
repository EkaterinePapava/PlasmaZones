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
 * @param {Object} params - Tiling parameters
 * @returns {Array<{x: number, y: number, width: number, height: number}>}
 */

// Uses injected deckLayout(area, count, focusedFraction, horizontal)

function calculateZones(params) {
    // Overlapping layout — innerGap intentionally ignored (zones overlap by design)
    const count = params.windowCount;
    const area = params.area;
    const focusedFraction = params.splitRatio;

    // KEEP IN SYNC with deck.js
    return deckLayout(area, count, focusedFraction, true);
}
