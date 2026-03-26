// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

// @name Focus + Sidebar
// @description Main window with vertically stacked sidebar; ratio controls main window width
// @producesOverlappingZones false
// @supportsMasterCount false
// @supportsSplitRatio true
// @defaultSplitRatio 0.7
// @defaultMaxWindows 5
// @minimumWindows 1
// @zoneNumberDisplay all

/**
 * Focus + Sidebar layout: one large main window on the left with a narrow
 * sidebar column on the right containing vertically stacked small windows.
 * splitRatio controls the main window width fraction (default 0.7 = 70%).
 *
 * @param {Object} params - Tiling parameters
 * @returns {Array<{x: number, y: number, width: number, height: number}>}
 */
function calculateZones(params) {
    var count = params.windowCount;
    var area = params.area;
    var gap = params.innerGap || 0;

    if (count <= 0) return [];
    if (count === 1) return [area];

    var splitRatio = params.splitRatio > 0 ? params.splitRatio : 0.7;
    var mainWidth = Math.round(area.width * splitRatio - gap / 2);
    var sidebarX = area.x + mainWidth + gap;
    var sidebarWidth = area.x + area.width - sidebarX;

    var zones = [];

    // Window 1: main window on the left
    zones.push({
        x: area.x,
        y: area.y,
        width: mainWidth,
        height: area.height
    });

    // Windows 2+: stack vertically in the right sidebar column
    var sidebarCount = count - 1;
    var totalGaps = (sidebarCount - 1) * gap;
    var windowHeight = Math.round((area.height - totalGaps) / sidebarCount);

    for (var i = 0; i < sidebarCount; i++) {
        var y = area.y + i * (windowHeight + gap);
        var h = (i === sidebarCount - 1)
            ? (area.y + area.height - y)
            : windowHeight;

        zones.push({
            x: sidebarX,
            y: y,
            width: sidebarWidth,
            height: h
        });
    }

    return zones;
}
