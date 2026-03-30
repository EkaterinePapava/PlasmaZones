// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColor>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QtCore/qnamespace.h>

#include "../core/constants.h"
#include "plasmazones_export.h"

namespace PlasmaZones {

/**
 * @brief Provides static access to default configuration values
 *
 * Canonical default values for all PlasmaZones configuration keys.
 * Used by Settings::load() when no persisted value exists.
 *
 * Usage:
 *   int cols = ConfigDefaults::gridColumns();  // Returns 5
 *   int rows = ConfigDefaults::maxRows();      // Returns 4
 */
class ConfigDefaults
{
public:
    // ═══════════════════════════════════════════════════════════════════════════
    // Activation Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static QVariantList dragActivationTriggers()
    {
        // Default: single trigger with Alt modifier (3), no mouse button
        QVariantMap trigger;
        trigger[QStringLiteral("modifier")] = 3;
        trigger[QStringLiteral("mouseButton")] = 0;
        return {trigger};
    }
    static bool toggleActivation()
    {
        return false;
    }
    static bool snappingEnabled()
    {
        return true;
    }
    static bool zoneSpanEnabled()
    {
        return true;
    }
    static int zoneSpanModifier()
    {
        return 2;
    }
    static QVariantList zoneSpanTriggers()
    {
        QVariantMap trigger;
        trigger[QStringLiteral("modifier")] = zoneSpanModifier();
        trigger[QStringLiteral("mouseButton")] = 0;
        return {trigger};
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Display Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static bool showOnAllMonitors()
    {
        return false;
    }
    static QStringList disabledDesktops()
    {
        return {};
    }
    static QStringList disabledActivities()
    {
        return {};
    }
    static bool showNumbers()
    {
        return true;
    }
    static bool flashOnSwitch()
    {
        return true;
    }
    static bool showOsdOnLayoutSwitch()
    {
        return true;
    }
    static bool showNavigationOsd()
    {
        return true;
    }
    static constexpr int osdStyleMin()
    {
        return 0;
    }
    static constexpr int osdStyleMax()
    {
        return 2;
    }
    static int osdStyle()
    {
        return 2;
    }
    static constexpr int overlayDisplayModeMin()
    {
        return 0;
    }
    static constexpr int overlayDisplayModeMax()
    {
        return 1;
    }
    static int overlayDisplayMode()
    {
        return 0;
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Appearance Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static bool useSystemColors()
    {
        return true;
    }
    static QColor highlightColor()
    {
        // #AARRGGBB: #800078D4 → A=0x80, R=0x00, G=0x78, B=0xD4
        return QColor(0x00, 0x78, 0xD4, 0x80);
    }
    static QColor inactiveColor()
    {
        // #40808080
        return QColor(0x80, 0x80, 0x80, 0x40);
    }
    static QColor borderColor()
    {
        // #C8FFFFFF
        return QColor(0xFF, 0xFF, 0xFF, 0xC8);
    }
    static QColor labelFontColor()
    {
        // #FFFFFFFF
        return QColor(0xFF, 0xFF, 0xFF, 0xFF);
    }
    static double activeOpacity()
    {
        return 0.5;
    }
    static constexpr qreal activeOpacityMin()
    {
        return 0.0;
    }
    static constexpr qreal activeOpacityMax()
    {
        return 1.0;
    }
    static double inactiveOpacity()
    {
        return 0.3;
    }
    static constexpr qreal inactiveOpacityMin()
    {
        return 0.0;
    }
    static constexpr qreal inactiveOpacityMax()
    {
        return 1.0;
    }
    static int borderWidth()
    {
        return 2;
    }
    static constexpr int borderWidthMin()
    {
        return 0;
    }
    static constexpr int borderWidthMax()
    {
        return 10;
    }
    static int borderRadius()
    {
        return 8;
    }
    static constexpr int borderRadiusMin()
    {
        return 0;
    }
    static constexpr int borderRadiusMax()
    {
        return 50;
    }
    static bool enableBlur()
    {
        return true;
    }
    static QString labelFontFamily()
    {
        return QString();
    }
    static double labelFontSizeScale()
    {
        return 1.0;
    }
    static constexpr qreal labelFontSizeScaleMin()
    {
        return 0.25;
    }
    static constexpr qreal labelFontSizeScaleMax()
    {
        return 3.0;
    }
    static int labelFontWeight()
    {
        return 700;
    }
    static constexpr int labelFontWeightMin()
    {
        return 100;
    }
    static constexpr int labelFontWeightMax()
    {
        return 900;
    }
    static bool labelFontItalic()
    {
        return false;
    }
    static bool labelFontUnderline()
    {
        return false;
    }
    static bool labelFontStrikeout()
    {
        return false;
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Zone Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static int zonePadding()
    {
        return 8;
    }
    static constexpr int zonePaddingMin()
    {
        return 0;
    }
    static constexpr int zonePaddingMax()
    {
        return 50;
    }
    static int outerGap()
    {
        return 8;
    }
    static constexpr int outerGapMin()
    {
        return 0;
    }
    static constexpr int outerGapMax()
    {
        return 50;
    }
    static bool usePerSideOuterGap()
    {
        return false;
    }
    static int outerGapTop()
    {
        return 8;
    }
    static constexpr int outerGapTopMin()
    {
        return 0;
    }
    static constexpr int outerGapTopMax()
    {
        return 50;
    }
    static int outerGapBottom()
    {
        return 8;
    }
    static constexpr int outerGapBottomMin()
    {
        return 0;
    }
    static constexpr int outerGapBottomMax()
    {
        return 50;
    }
    static int outerGapLeft()
    {
        return 8;
    }
    static constexpr int outerGapLeftMin()
    {
        return 0;
    }
    static constexpr int outerGapLeftMax()
    {
        return 50;
    }
    static int outerGapRight()
    {
        return 8;
    }
    static constexpr int outerGapRightMin()
    {
        return 0;
    }
    static constexpr int outerGapRightMax()
    {
        return 50;
    }
    static int adjacentThreshold()
    {
        return 20;
    }
    static constexpr int adjacentThresholdMin()
    {
        return 5;
    }
    static constexpr int adjacentThresholdMax()
    {
        return 500;
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Performance Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static int pollIntervalMs()
    {
        return 50;
    }
    static constexpr int pollIntervalMsMin()
    {
        return 10;
    }
    static constexpr int pollIntervalMsMax()
    {
        return 1000;
    }
    static int minimumZoneSizePx()
    {
        return 100;
    }
    static constexpr int minimumZoneSizePxMin()
    {
        return 50;
    }
    static constexpr int minimumZoneSizePxMax()
    {
        return 500;
    }
    static int minimumZoneDisplaySizePx()
    {
        return 10;
    }
    static constexpr int minimumZoneDisplaySizePxMin()
    {
        return 1;
    }
    static constexpr int minimumZoneDisplaySizePxMax()
    {
        return 50;
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Window Behavior Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static bool keepWindowsInZonesOnResolutionChange()
    {
        return true;
    }
    static bool moveNewWindowsToLastZone()
    {
        return false;
    }
    static bool restoreOriginalSizeOnUnsnap()
    {
        return true;
    }
    static int stickyWindowHandling()
    {
        return 0;
    }
    static bool restoreWindowsToZonesOnLogin()
    {
        return true;
    }
    static bool filterLayoutsByAspectRatio()
    {
        return true;
    }
    static bool snapAssistFeatureEnabled()
    {
        return true;
    }
    static bool snapAssistEnabled()
    {
        return true;
    }
    static QVariantList snapAssistTriggers()
    {
        // Default: Middle mouse
        QVariantMap trigger;
        trigger[QStringLiteral("modifier")] = 0;
        trigger[QStringLiteral("mouseButton")] = static_cast<int>(Qt::MiddleButton);
        return {trigger};
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Exclusion Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static bool excludeTransientWindows()
    {
        return true;
    }
    static int minimumWindowWidth()
    {
        return 200;
    }
    static constexpr int minimumWindowWidthMin()
    {
        return 0;
    }
    static constexpr int minimumWindowWidthMax()
    {
        return 2000;
    }
    static int minimumWindowHeight()
    {
        return 150;
    }
    static constexpr int minimumWindowHeightMin()
    {
        return 0;
    }
    static constexpr int minimumWindowHeightMax()
    {
        return 2000;
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Zone Selector Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static bool zoneSelectorEnabled()
    {
        return true;
    }
    static int triggerDistance()
    {
        return 50;
    }
    static constexpr int triggerDistanceMin()
    {
        return 10;
    }
    static constexpr int triggerDistanceMax()
    {
        return 200;
    }
    static int position()
    {
        return 1;
    }
    static int layoutMode()
    {
        return 0;
    }
    static int sizeMode()
    {
        return 0;
    }
    static int maxRows()
    {
        return 4;
    }
    static constexpr int maxRowsMin()
    {
        return 1;
    }
    static constexpr int maxRowsMax()
    {
        return 10;
    }
    static int previewWidth()
    {
        return 180;
    }
    static constexpr int previewWidthMin()
    {
        return 80;
    }
    static constexpr int previewWidthMax()
    {
        return 400;
    }
    static int previewHeight()
    {
        return 101;
    }
    static constexpr int previewHeightMin()
    {
        return 60;
    }
    static constexpr int previewHeightMax()
    {
        return 300;
    }
    static bool previewLockAspect()
    {
        return true;
    }
    static int gridColumns()
    {
        return 5;
    }
    static constexpr int gridColumnsMin()
    {
        return 1;
    }
    static constexpr int gridColumnsMax()
    {
        return 10;
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Group Names
    // ═══════════════════════════════════════════════════════════════════════════

    static QString generalGroup()
    {
        return QStringLiteral("General");
    }
    static QString activationGroup()
    {
        return QStringLiteral("Activation");
    }
    static QString displayGroup()
    {
        return QStringLiteral("Display");
    }
    static QString appearanceGroup()
    {
        return QStringLiteral("Appearance");
    }
    static QString zonesGroup()
    {
        return QStringLiteral("Zones");
    }
    static QString behaviorGroup()
    {
        return QStringLiteral("Behavior");
    }
    static QString exclusionsGroup()
    {
        return QStringLiteral("Exclusions");
    }
    static QString zoneSelectorGroup()
    {
        return QStringLiteral("ZoneSelector");
    }
    static QString shadersGroup()
    {
        return QStringLiteral("Shaders");
    }
    static QString globalShortcutsGroup()
    {
        return QStringLiteral("GlobalShortcuts");
    }
    static QString autotilingGroup()
    {
        return QStringLiteral("Autotiling");
    }
    static QString autotileShortcutsGroup()
    {
        return QStringLiteral("AutotileShortcuts");
    }
    static QString animationsGroup()
    {
        return QStringLiteral("Animations");
    }
    static QString updatesGroup()
    {
        return QStringLiteral("Updates");
    }
    static QString editorGroup()
    {
        return QStringLiteral("Editor");
    }
    static QString tilingQuickLayoutSlotsGroup()
    {
        return QStringLiteral("TilingQuickLayoutSlots");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — General
    // ═══════════════════════════════════════════════════════════════════════════

    static QString renderingBackendKey()
    {
        return QStringLiteral("RenderingBackend");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — Activation
    // ═══════════════════════════════════════════════════════════════════════════

    static QString dragActivationTriggersKey()
    {
        return QStringLiteral("DragActivationTriggers");
    }
    static QString zoneSpanEnabledKey()
    {
        return QStringLiteral("ZoneSpanEnabled");
    }
    static QString zoneSpanModifierKey()
    {
        return QStringLiteral("ZoneSpanModifier");
    }
    static QString zoneSpanTriggersKey()
    {
        return QStringLiteral("ZoneSpanTriggers");
    }
    static QString toggleActivationKey()
    {
        return QStringLiteral("ToggleActivation");
    }
    static QString snappingEnabledKey()
    {
        return QStringLiteral("SnappingEnabled");
    }
    static QString snapAssistFeatureEnabledKey()
    {
        return QStringLiteral("SnapAssistFeatureEnabled");
    }
    static QString snapAssistEnabledKey()
    {
        return QStringLiteral("SnapAssistEnabled");
    }
    static QString snapAssistTriggersKey()
    {
        return QStringLiteral("SnapAssistTriggers");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — Display
    // ═══════════════════════════════════════════════════════════════════════════

    static QString showOnAllMonitorsKey()
    {
        return QStringLiteral("ShowOnAllMonitors");
    }
    static QString disabledMonitorsKey()
    {
        return QStringLiteral("DisabledMonitors");
    }
    static QString disabledDesktopsKey()
    {
        return QStringLiteral("DisabledDesktops");
    }
    static QString disabledActivitiesKey()
    {
        return QStringLiteral("DisabledActivities");
    }
    static QString showNumbersKey()
    {
        return QStringLiteral("ShowNumbers");
    }
    static QString flashOnSwitchKey()
    {
        return QStringLiteral("FlashOnSwitch");
    }
    static QString showOsdOnLayoutSwitchKey()
    {
        return QStringLiteral("ShowOsdOnLayoutSwitch");
    }
    static QString showNavigationOsdKey()
    {
        return QStringLiteral("ShowNavigationOsd");
    }
    static QString osdStyleKey()
    {
        return QStringLiteral("OsdStyle");
    }
    static QString overlayDisplayModeKey()
    {
        return QStringLiteral("OverlayDisplayMode");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — Appearance
    // ═══════════════════════════════════════════════════════════════════════════

    static QString useSystemColorsKey()
    {
        return QStringLiteral("UseSystemColors");
    }
    static QString highlightColorKey()
    {
        return QStringLiteral("HighlightColor");
    }
    static QString inactiveColorKey()
    {
        return QStringLiteral("InactiveColor");
    }
    static QString borderColorKey()
    {
        return QStringLiteral("BorderColor");
    }
    static QString labelFontColorKey()
    {
        return QStringLiteral("LabelFontColor");
    }
    static QString activeOpacityKey()
    {
        return QStringLiteral("ActiveOpacity");
    }
    static QString inactiveOpacityKey()
    {
        return QStringLiteral("InactiveOpacity");
    }
    static QString borderWidthKey()
    {
        return QStringLiteral("BorderWidth");
    }
    static QString borderRadiusKey()
    {
        return QStringLiteral("BorderRadius");
    }
    static QString enableBlurKey()
    {
        return QStringLiteral("EnableBlur");
    }
    static QString labelFontFamilyKey()
    {
        return QStringLiteral("LabelFontFamily");
    }
    static QString labelFontSizeScaleKey()
    {
        return QStringLiteral("LabelFontSizeScale");
    }
    static QString labelFontWeightKey()
    {
        return QStringLiteral("LabelFontWeight");
    }
    static QString labelFontItalicKey()
    {
        return QStringLiteral("LabelFontItalic");
    }
    static QString labelFontUnderlineKey()
    {
        return QStringLiteral("LabelFontUnderline");
    }
    static QString labelFontStrikeoutKey()
    {
        return QStringLiteral("LabelFontStrikeout");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — Zones
    // ═══════════════════════════════════════════════════════════════════════════

    static QString zonePaddingKey()
    {
        return QStringLiteral("Padding");
    }
    static QString outerGapKey()
    {
        return QStringLiteral("OuterGap");
    }
    static QString usePerSideOuterGapKey()
    {
        return QStringLiteral("UsePerSideOuterGap");
    }
    static QString outerGapTopKey()
    {
        return QStringLiteral("OuterGapTop");
    }
    static QString outerGapBottomKey()
    {
        return QStringLiteral("OuterGapBottom");
    }
    static QString outerGapLeftKey()
    {
        return QStringLiteral("OuterGapLeft");
    }
    static QString outerGapRightKey()
    {
        return QStringLiteral("OuterGapRight");
    }
    static QString adjacentThresholdKey()
    {
        return QStringLiteral("AdjacentThreshold");
    }
    static QString pollIntervalMsKey()
    {
        return QStringLiteral("PollIntervalMs");
    }
    static QString minimumZoneSizePxKey()
    {
        return QStringLiteral("MinimumZoneSizePx");
    }
    static QString minimumZoneDisplaySizePxKey()
    {
        return QStringLiteral("MinimumZoneDisplaySizePx");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — Behavior
    // ═══════════════════════════════════════════════════════════════════════════

    static QString keepOnResolutionChangeKey()
    {
        return QStringLiteral("KeepOnResolutionChange");
    }
    static QString moveNewToLastZoneKey()
    {
        return QStringLiteral("MoveNewToLastZone");
    }
    static QString restoreSizeOnUnsnapKey()
    {
        return QStringLiteral("RestoreSizeOnUnsnap");
    }
    static QString stickyWindowHandlingKey()
    {
        return QStringLiteral("StickyWindowHandling");
    }
    static QString restoreWindowsToZonesOnLoginKey()
    {
        return QStringLiteral("RestoreWindowsToZonesOnLogin");
    }
    static QString defaultLayoutIdKey()
    {
        return QStringLiteral("DefaultLayoutId");
    }
    static QString filterLayoutsByAspectRatioKey()
    {
        return QStringLiteral("FilterLayoutsByAspectRatio");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — Exclusions
    // ═══════════════════════════════════════════════════════════════════════════

    static QString excludedApplicationsKey()
    {
        return QStringLiteral("Applications");
    }
    static QString excludedWindowClassesKey()
    {
        return QStringLiteral("WindowClasses");
    }
    static QString excludeTransientWindowsKey()
    {
        return QStringLiteral("ExcludeTransientWindows");
    }
    static QString minimumWindowWidthKey()
    {
        return QStringLiteral("MinimumWindowWidth");
    }
    static QString minimumWindowHeightKey()
    {
        return QStringLiteral("MinimumWindowHeight");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — ZoneSelector
    // ═══════════════════════════════════════════════════════════════════════════

    static QString zoneSelectorEnabledKey()
    {
        return QStringLiteral("Enabled");
    }
    static QString zoneSelectorTriggerDistanceKey()
    {
        return QStringLiteral("TriggerDistance");
    }
    static QString zoneSelectorPositionKey()
    {
        return QStringLiteral("Position");
    }
    static QString zoneSelectorLayoutModeKey()
    {
        return QStringLiteral("LayoutMode");
    }
    static QString zoneSelectorPreviewWidthKey()
    {
        return QStringLiteral("PreviewWidth");
    }
    static QString zoneSelectorPreviewHeightKey()
    {
        return QStringLiteral("PreviewHeight");
    }
    static QString zoneSelectorPreviewLockAspectKey()
    {
        return QStringLiteral("PreviewLockAspect");
    }
    static QString zoneSelectorGridColumnsKey()
    {
        return QStringLiteral("GridColumns");
    }
    static QString zoneSelectorSizeModeKey()
    {
        return QStringLiteral("SizeMode");
    }
    static QString zoneSelectorMaxRowsKey()
    {
        return QStringLiteral("MaxRows");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — Shaders
    // ═══════════════════════════════════════════════════════════════════════════

    static QString enableShaderEffectsKey()
    {
        return QStringLiteral("EnableShaderEffects");
    }
    static QString shaderFrameRateKey()
    {
        return QStringLiteral("ShaderFrameRate");
    }
    static QString enableAudioVisualizerKey()
    {
        return QStringLiteral("EnableAudioVisualizer");
    }
    static QString audioSpectrumBarCountKey()
    {
        return QStringLiteral("AudioSpectrumBarCount");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — GlobalShortcuts
    // ═══════════════════════════════════════════════════════════════════════════

    static QString openEditorShortcutKey()
    {
        return QStringLiteral("OpenEditorShortcut");
    }
    static QString openSettingsShortcutKey()
    {
        return QStringLiteral("OpenSettingsShortcut");
    }
    static QString previousLayoutShortcutKey()
    {
        return QStringLiteral("PreviousLayoutShortcut");
    }
    static QString nextLayoutShortcutKey()
    {
        return QStringLiteral("NextLayoutShortcut");
    }
    static QString quickLayoutShortcutKey(int n)
    {
        return QStringLiteral("QuickLayout%1Shortcut").arg(n);
    }
    static QString moveWindowLeftShortcutKey()
    {
        return QStringLiteral("MoveWindowLeft");
    }
    static QString moveWindowRightShortcutKey()
    {
        return QStringLiteral("MoveWindowRight");
    }
    static QString moveWindowUpShortcutKey()
    {
        return QStringLiteral("MoveWindowUp");
    }
    static QString moveWindowDownShortcutKey()
    {
        return QStringLiteral("MoveWindowDown");
    }
    static QString focusZoneLeftShortcutKey()
    {
        return QStringLiteral("FocusZoneLeft");
    }
    static QString focusZoneRightShortcutKey()
    {
        return QStringLiteral("FocusZoneRight");
    }
    static QString focusZoneUpShortcutKey()
    {
        return QStringLiteral("FocusZoneUp");
    }
    static QString focusZoneDownShortcutKey()
    {
        return QStringLiteral("FocusZoneDown");
    }
    static QString pushToEmptyZoneShortcutKey()
    {
        return QStringLiteral("PushToEmptyZone");
    }
    static QString restoreWindowSizeShortcutKey()
    {
        return QStringLiteral("RestoreWindowSize");
    }
    static QString toggleWindowFloatShortcutKey()
    {
        return QStringLiteral("ToggleWindowFloat");
    }
    static QString swapWindowLeftShortcutKey()
    {
        return QStringLiteral("SwapWindowLeft");
    }
    static QString swapWindowRightShortcutKey()
    {
        return QStringLiteral("SwapWindowRight");
    }
    static QString swapWindowUpShortcutKey()
    {
        return QStringLiteral("SwapWindowUp");
    }
    static QString swapWindowDownShortcutKey()
    {
        return QStringLiteral("SwapWindowDown");
    }
    static QString snapToZoneShortcutKey(int n)
    {
        return QStringLiteral("SnapToZone%1").arg(n);
    }
    static QString rotateWindowsClockwiseShortcutKey()
    {
        return QStringLiteral("RotateWindowsClockwise");
    }
    static QString rotateWindowsCounterclockwiseShortcutKey()
    {
        return QStringLiteral("RotateWindowsCounterclockwise");
    }
    static QString cycleWindowForwardShortcutKey()
    {
        return QStringLiteral("CycleWindowForward");
    }
    static QString cycleWindowBackwardShortcutKey()
    {
        return QStringLiteral("CycleWindowBackward");
    }
    static QString resnapToNewLayoutShortcutKey()
    {
        return QStringLiteral("ResnapToNewLayoutShortcut");
    }
    static QString snapAllWindowsShortcutKey()
    {
        return QStringLiteral("SnapAllWindowsShortcut");
    }
    static QString layoutPickerShortcutKey()
    {
        return QStringLiteral("LayoutPickerShortcut");
    }
    static QString toggleLayoutLockShortcutKey()
    {
        return QStringLiteral("ToggleLayoutLockShortcut");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — Autotiling
    // ═══════════════════════════════════════════════════════════════════════════

    static QString autotileEnabledKey()
    {
        return QStringLiteral("AutotileEnabled");
    }
    static QString defaultAutotileAlgorithmKey()
    {
        return QStringLiteral("DefaultAutotileAlgorithm");
    }
    static QString autotileSplitRatioKey()
    {
        return QStringLiteral("AutotileSplitRatio");
    }
    static QString autotileMasterCountKey()
    {
        return QStringLiteral("AutotileMasterCount");
    }
    static QString autotilePerAlgorithmSettingsKey()
    {
        return QStringLiteral("AutotilePerAlgorithmSettings");
    }
    static QString autotileInnerGapKey()
    {
        return QStringLiteral("AutotileInnerGap");
    }
    static QString autotileOuterGapKey()
    {
        return QStringLiteral("AutotileOuterGap");
    }
    static QString autotileUsePerSideOuterGapKey()
    {
        return QStringLiteral("AutotileUsePerSideOuterGap");
    }
    static QString autotileOuterGapTopKey()
    {
        return QStringLiteral("AutotileOuterGapTop");
    }
    static QString autotileOuterGapBottomKey()
    {
        return QStringLiteral("AutotileOuterGapBottom");
    }
    static QString autotileOuterGapLeftKey()
    {
        return QStringLiteral("AutotileOuterGapLeft");
    }
    static QString autotileOuterGapRightKey()
    {
        return QStringLiteral("AutotileOuterGapRight");
    }
    static QString autotileFocusNewWindowsKey()
    {
        return QStringLiteral("AutotileFocusNewWindows");
    }
    static QString autotileSmartGapsKey()
    {
        return QStringLiteral("AutotileSmartGaps");
    }
    static QString autotileMaxWindowsKey()
    {
        return QStringLiteral("AutotileMaxWindows");
    }
    static QString autotileInsertPositionKey()
    {
        return QStringLiteral("AutotileInsertPosition");
    }
    static QString autotileFocusFollowsMouseKey()
    {
        return QStringLiteral("AutotileFocusFollowsMouse");
    }
    static QString autotileRespectMinimumSizeKey()
    {
        return QStringLiteral("AutotileRespectMinimumSize");
    }
    static QString autotileHideTitleBarsKey()
    {
        return QStringLiteral("AutotileHideTitleBars");
    }
    static QString autotileShowBorderKey()
    {
        return QStringLiteral("AutotileShowBorder");
    }
    static QString autotileBorderWidthKey()
    {
        return QStringLiteral("AutotileBorderWidth");
    }
    static QString autotileBorderRadiusKey()
    {
        return QStringLiteral("AutotileBorderRadius");
    }
    static QString autotileBorderColorKey()
    {
        return QStringLiteral("AutotileBorderColor");
    }
    static QString autotileInactiveBorderColorKey()
    {
        return QStringLiteral("AutotileInactiveBorderColor");
    }
    static QString autotileUseSystemBorderColorsKey()
    {
        return QStringLiteral("AutotileUseSystemBorderColors");
    }
    static QString lockedScreensKey()
    {
        return QStringLiteral("LockedScreens");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — AutotileShortcuts
    // ═══════════════════════════════════════════════════════════════════════════

    static QString autotileToggleShortcutKey()
    {
        return QStringLiteral("ToggleShortcut");
    }
    static QString autotileFocusMasterShortcutKey()
    {
        return QStringLiteral("FocusMasterShortcut");
    }
    static QString autotileSwapMasterShortcutKey()
    {
        return QStringLiteral("SwapMasterShortcut");
    }
    static QString autotileIncMasterRatioShortcutKey()
    {
        return QStringLiteral("IncMasterRatioShortcut");
    }
    static QString autotileDecMasterRatioShortcutKey()
    {
        return QStringLiteral("DecMasterRatioShortcut");
    }
    static QString autotileIncMasterCountShortcutKey()
    {
        return QStringLiteral("IncMasterCountShortcut");
    }
    static QString autotileDecMasterCountShortcutKey()
    {
        return QStringLiteral("DecMasterCountShortcut");
    }
    static QString autotileRetileShortcutKey()
    {
        return QStringLiteral("RetileShortcut");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — Animations
    // ═══════════════════════════════════════════════════════════════════════════

    static QString animationsEnabledKey()
    {
        return QStringLiteral("AnimationsEnabled");
    }
    static QString animationDurationKey()
    {
        return QStringLiteral("AnimationDuration");
    }
    static QString animationEasingCurveKey()
    {
        return QStringLiteral("AnimationEasingCurve");
    }
    static QString animationMinDistanceKey()
    {
        return QStringLiteral("AnimationMinDistance");
    }
    static QString animationSequenceModeKey()
    {
        return QStringLiteral("AnimationSequenceMode");
    }
    static QString animationStaggerIntervalKey()
    {
        return QStringLiteral("AnimationStaggerInterval");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Keys — Editor
    // ═══════════════════════════════════════════════════════════════════════════

    static QString editorDuplicateShortcutKey()
    {
        return QStringLiteral("EditorDuplicateShortcut");
    }
    static QString editorSplitHorizontalShortcutKey()
    {
        return QStringLiteral("EditorSplitHorizontalShortcut");
    }
    static QString editorSplitVerticalShortcutKey()
    {
        return QStringLiteral("EditorSplitVerticalShortcut");
    }
    static QString editorFillShortcutKey()
    {
        return QStringLiteral("EditorFillShortcut");
    }
    static QString editorGridSnappingEnabledKey()
    {
        return QStringLiteral("GridSnappingEnabled");
    }
    static QString editorEdgeSnappingEnabledKey()
    {
        return QStringLiteral("EdgeSnappingEnabled");
    }
    static QString editorSnapIntervalXKey()
    {
        return QStringLiteral("SnapIntervalX");
    }
    static QString editorSnapIntervalYKey()
    {
        return QStringLiteral("SnapIntervalY");
    }
    static QString editorSnapIntervalKey()
    {
        return QStringLiteral("SnapInterval");
    }
    static QString editorSnapOverrideModifierKey()
    {
        return QStringLiteral("SnapOverrideModifier");
    }
    static QString fillOnDropEnabledKey()
    {
        return QStringLiteral("FillOnDropEnabled");
    }
    static QString fillOnDropModifierKey()
    {
        return QStringLiteral("FillOnDropModifier");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config Path
    // ═══════════════════════════════════════════════════════════════════════════

    // Returns the absolute path to plasmazonesrc.
    // Not cached — QStandardPaths respects $XDG_CONFIG_HOME changes at runtime,
    // which tests rely on via IsolatedConfigGuard.
    PLASMAZONES_EXPORT static QString configFilePath();

    // ═══════════════════════════════════════════════════════════════════════════
    // Rendering Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static QString renderingBackend()
    {
        return QStringLiteral("auto");
    }

    struct RenderingBackendEntry
    {
        QString key;
        QString displayName;
    };

    // Single source of truth for backend keys and display names.
    // Order here determines ComboBox order in the settings UI.
    // When adding entries, also add the display name to the translation catalog.
    static const QList<RenderingBackendEntry>& renderingBackendEntries()
    {
        static const QList<RenderingBackendEntry> entries = {
            {QStringLiteral("auto"), QStringLiteral("Automatic")},
            {QStringLiteral("vulkan"), QStringLiteral("Vulkan")},
            {QStringLiteral("opengl"), QStringLiteral("OpenGL")},
        };
        return entries;
    }

    static const QStringList& renderingBackendOptions()
    {
        static const QStringList keys = [] {
            QStringList k;
            for (const auto& e : renderingBackendEntries())
                k.append(e.key);
            return k;
        }();
        return keys;
    }

    // Untranslated display names — use for translation source only.
    // SettingsController translates these via PzI18n::tr() at runtime.
    static QStringList renderingBackendDisplayNames()
    {
        QStringList names;
        for (const auto& e : renderingBackendEntries())
            names.append(e.displayName);
        return names;
    }

    static QString normalizeRenderingBackend(const QString& raw)
    {
        const QString normalized = raw.toLower().trimmed();
        for (const auto& e : renderingBackendEntries()) {
            if (e.key == normalized)
                return normalized;
        }
        return renderingBackend();
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Shader Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static bool enableShaderEffects()
    {
        return true;
    }
    static int shaderFrameRate()
    {
        return 60;
    }
    static constexpr int shaderFrameRateMin()
    {
        return 30;
    }
    static constexpr int shaderFrameRateMax()
    {
        return 144;
    }
    static bool enableAudioVisualizer()
    {
        return false;
    }
    static int audioSpectrumBarCount()
    {
        return 64;
    }
    static constexpr int audioSpectrumBarCountMin()
    {
        return 16;
    }
    static constexpr int audioSpectrumBarCountMax()
    {
        return 256;
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Autotile Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static bool autotileEnabled()
    {
        return false;
    }
    static QString defaultAutotileAlgorithm()
    {
        return QStringLiteral("bsp");
    }
    static double autotileSplitRatio()
    {
        return 0.5;
    }
    static constexpr qreal autotileSplitRatioMin()
    {
        return AutotileDefaults::MinSplitRatio;
    }
    static constexpr qreal autotileSplitRatioMax()
    {
        return AutotileDefaults::MaxSplitRatio;
    }
    static int autotileMasterCount()
    {
        return 1;
    }
    static constexpr int autotileMasterCountMin()
    {
        return AutotileDefaults::MinMasterCount;
    }
    static constexpr int autotileMasterCountMax()
    {
        return AutotileDefaults::MaxMasterCount;
    }
    static int autotileInnerGap()
    {
        return 8;
    }
    static constexpr int autotileInnerGapMin()
    {
        return AutotileDefaults::MinGap;
    }
    static constexpr int autotileInnerGapMax()
    {
        return AutotileDefaults::MaxGap;
    }
    static int autotileOuterGap()
    {
        return 8;
    }
    static constexpr int autotileOuterGapMin()
    {
        return AutotileDefaults::MinGap;
    }
    static constexpr int autotileOuterGapMax()
    {
        return AutotileDefaults::MaxGap;
    }
    static bool autotileUsePerSideOuterGap()
    {
        return false;
    }
    static int autotileOuterGapTop()
    {
        return 8;
    }
    static constexpr int autotileOuterGapTopMin()
    {
        return AutotileDefaults::MinGap;
    }
    static constexpr int autotileOuterGapTopMax()
    {
        return AutotileDefaults::MaxGap;
    }
    static int autotileOuterGapBottom()
    {
        return 8;
    }
    static constexpr int autotileOuterGapBottomMin()
    {
        return AutotileDefaults::MinGap;
    }
    static constexpr int autotileOuterGapBottomMax()
    {
        return AutotileDefaults::MaxGap;
    }
    static int autotileOuterGapLeft()
    {
        return 8;
    }
    static constexpr int autotileOuterGapLeftMin()
    {
        return AutotileDefaults::MinGap;
    }
    static constexpr int autotileOuterGapLeftMax()
    {
        return AutotileDefaults::MaxGap;
    }
    static int autotileOuterGapRight()
    {
        return 8;
    }
    static constexpr int autotileOuterGapRightMin()
    {
        return AutotileDefaults::MinGap;
    }
    static constexpr int autotileOuterGapRightMax()
    {
        return AutotileDefaults::MaxGap;
    }
    static bool autotileFocusNewWindows()
    {
        return true;
    }
    static bool autotileSmartGaps()
    {
        return true;
    }
    static int autotileInsertPosition()
    {
        return 0;
    }
    static constexpr int autotileInsertPositionMin()
    {
        return 0;
    }
    static constexpr int autotileInsertPositionMax()
    {
        return 2;
    }
    static int autotileMaxWindows()
    {
        return 5;
    }
    static constexpr int autotileMaxWindowsMin()
    {
        return AutotileDefaults::MinMaxWindows;
    }
    static constexpr int autotileMaxWindowsMax()
    {
        return AutotileDefaults::MaxMaxWindows;
    }
    static bool animationsEnabled()
    {
        return true;
    }
    static int animationDuration()
    {
        return 300;
    }
    static constexpr int animationDurationMin()
    {
        return 50;
    }
    static constexpr int animationDurationMax()
    {
        return 500;
    }
    static int animationSequenceMode()
    {
        return 1;
    }
    static constexpr int animationSequenceModeMin()
    {
        return 0;
    }
    static constexpr int animationSequenceModeMax()
    {
        return 1;
    }
    static int animationStaggerInterval()
    {
        return 50;
    }
    static constexpr int animationStaggerIntervalMin()
    {
        return AutotileDefaults::MinAnimationStaggerIntervalMs;
    }
    static constexpr int animationStaggerIntervalMax()
    {
        return AutotileDefaults::MaxAnimationStaggerIntervalMs;
    }
    static QString animationEasingCurve()
    {
        return QStringLiteral("0.33,1.00,0.68,1.00");
    }
    static int animationMinDistance()
    {
        return 0;
    }
    static constexpr int animationMinDistanceMin()
    {
        return 0;
    }
    static constexpr int animationMinDistanceMax()
    {
        return 200;
    }
    static bool autotileFocusFollowsMouse()
    {
        return false;
    }
    static bool autotileRespectMinimumSize()
    {
        return true;
    }
    static bool autotileHideTitleBars()
    {
        return true;
    }
    static bool autotileShowBorder()
    {
        return true;
    }
    static int autotileBorderWidth()
    {
        return 2;
    }
    static constexpr int autotileBorderWidthMin()
    {
        return 0;
    }
    static constexpr int autotileBorderWidthMax()
    {
        return 10;
    }
    static int autotileBorderRadius()
    {
        return 0;
    }
    static constexpr int autotileBorderRadiusMin()
    {
        return 0;
    }
    static constexpr int autotileBorderRadiusMax()
    {
        return 20;
    }
    static QColor autotileBorderColor()
    {
        // #800078D4
        return QColor(0x00, 0x78, 0xD4, 0x80);
    }
    static QColor autotileInactiveBorderColor()
    {
        // #40808080
        return QColor(0x80, 0x80, 0x80, 0x40);
    }
    static bool autotileUseSystemBorderColors()
    {
        return true;
    }
    static QStringList lockedScreens()
    {
        return {};
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Editor Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static QString editorDuplicateShortcut()
    {
        return QStringLiteral("Ctrl+D");
    }
    static QString editorSplitHorizontalShortcut()
    {
        return QStringLiteral("Ctrl+Shift+H");
    }
    static QString editorSplitVerticalShortcut()
    {
        return QStringLiteral("Ctrl+Alt+V");
    }
    static QString editorFillShortcut()
    {
        return QStringLiteral("Ctrl+Shift+F");
    }
    static bool editorGridSnappingEnabled()
    {
        return true;
    }
    static bool editorEdgeSnappingEnabled()
    {
        return true;
    }
    static double editorSnapInterval()
    {
        return 0.1;
    }
    static int editorSnapOverrideModifier()
    {
        return static_cast<int>(Qt::ShiftModifier);
    }
    static bool fillOnDropEnabled()
    {
        return true;
    }
    static int fillOnDropModifier()
    {
        return static_cast<int>(Qt::ControlModifier);
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Update Notification Settings
    // ═══════════════════════════════════════════════════════════════════════════

    static QString dismissedUpdateVersion()
    {
        return QString();
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Global Shortcuts
    // ═══════════════════════════════════════════════════════════════════════════

    static QString openEditorShortcut()
    {
        return QStringLiteral("Meta+Shift+E");
    }
    static QString openSettingsShortcut()
    {
        return QStringLiteral("Meta+Shift+P");
    }
    static QString previousLayoutShortcut()
    {
        return QStringLiteral("Meta+Alt+[");
    }
    static QString nextLayoutShortcut()
    {
        return QStringLiteral("Meta+Alt+]");
    }
    static QString quickLayout1Shortcut()
    {
        return QStringLiteral("Meta+Alt+1");
    }
    static QString quickLayout2Shortcut()
    {
        return QStringLiteral("Meta+Alt+2");
    }
    static QString quickLayout3Shortcut()
    {
        return QStringLiteral("Meta+Alt+3");
    }
    static QString quickLayout4Shortcut()
    {
        return QStringLiteral("Meta+Alt+4");
    }
    static QString quickLayout5Shortcut()
    {
        return QStringLiteral("Meta+Alt+5");
    }
    static QString quickLayout6Shortcut()
    {
        return QStringLiteral("Meta+Alt+6");
    }
    static QString quickLayout7Shortcut()
    {
        return QStringLiteral("Meta+Alt+7");
    }
    static QString quickLayout8Shortcut()
    {
        return QStringLiteral("Meta+Alt+8");
    }
    static QString quickLayout9Shortcut()
    {
        return QStringLiteral("Meta+Alt+9");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Navigation Shortcuts
    // ═══════════════════════════════════════════════════════════════════════════

    static QString moveWindowLeftShortcut()
    {
        return QStringLiteral("Meta+Alt+Shift+Left");
    }
    static QString moveWindowRightShortcut()
    {
        return QStringLiteral("Meta+Alt+Shift+Right");
    }
    static QString moveWindowUpShortcut()
    {
        return QStringLiteral("Meta+Alt+Shift+Up");
    }
    static QString moveWindowDownShortcut()
    {
        return QStringLiteral("Meta+Alt+Shift+Down");
    }
    static QString swapWindowLeftShortcut()
    {
        return QStringLiteral("Meta+Ctrl+Alt+Left");
    }
    static QString swapWindowRightShortcut()
    {
        return QStringLiteral("Meta+Ctrl+Alt+Right");
    }
    static QString swapWindowUpShortcut()
    {
        return QStringLiteral("Meta+Ctrl+Alt+Up");
    }
    static QString swapWindowDownShortcut()
    {
        return QStringLiteral("Meta+Ctrl+Alt+Down");
    }
    static QString focusZoneLeftShortcut()
    {
        return QStringLiteral("Alt+Shift+Left");
    }
    static QString focusZoneRightShortcut()
    {
        return QStringLiteral("Alt+Shift+Right");
    }
    static QString focusZoneUpShortcut()
    {
        return QStringLiteral("Alt+Shift+Up");
    }
    static QString focusZoneDownShortcut()
    {
        return QStringLiteral("Alt+Shift+Down");
    }
    static QString pushToEmptyZoneShortcut()
    {
        return QStringLiteral("Meta+Alt+Return");
    }
    static QString restoreWindowSizeShortcut()
    {
        return QStringLiteral("Meta+Alt+Escape");
    }
    static QString toggleWindowFloatShortcut()
    {
        return QStringLiteral("Meta+F");
    }
    static QString snapToZone1Shortcut()
    {
        return QStringLiteral("Meta+Ctrl+1");
    }
    static QString snapToZone2Shortcut()
    {
        return QStringLiteral("Meta+Ctrl+2");
    }
    static QString snapToZone3Shortcut()
    {
        return QStringLiteral("Meta+Ctrl+3");
    }
    static QString snapToZone4Shortcut()
    {
        return QStringLiteral("Meta+Ctrl+4");
    }
    static QString snapToZone5Shortcut()
    {
        return QStringLiteral("Meta+Ctrl+5");
    }
    static QString snapToZone6Shortcut()
    {
        return QStringLiteral("Meta+Ctrl+6");
    }
    static QString snapToZone7Shortcut()
    {
        return QStringLiteral("Meta+Ctrl+7");
    }
    static QString snapToZone8Shortcut()
    {
        return QStringLiteral("Meta+Ctrl+8");
    }
    static QString snapToZone9Shortcut()
    {
        return QStringLiteral("Meta+Ctrl+9");
    }
    static QString rotateWindowsClockwiseShortcut()
    {
        return QStringLiteral("Meta+Ctrl+]");
    }
    static QString rotateWindowsCounterclockwiseShortcut()
    {
        return QStringLiteral("Meta+Ctrl+[");
    }
    static QString cycleWindowForwardShortcut()
    {
        return QStringLiteral("Meta+Alt+.");
    }
    static QString cycleWindowBackwardShortcut()
    {
        return QStringLiteral("Meta+Alt+,");
    }
    static QString resnapToNewLayoutShortcut()
    {
        return QStringLiteral("Meta+Ctrl+Z");
    }
    static QString snapAllWindowsShortcut()
    {
        return QStringLiteral("Meta+Ctrl+S");
    }
    static QString layoutPickerShortcut()
    {
        return QStringLiteral("Meta+Alt+Space");
    }
    static QString toggleLayoutLockShortcut()
    {
        return QStringLiteral("Meta+Ctrl+L");
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Autotile Shortcuts
    // ═══════════════════════════════════════════════════════════════════════════

    static QString autotileToggleShortcut()
    {
        return QStringLiteral("Meta+Shift+T");
    }
    static QString autotileFocusMasterShortcut()
    {
        return QStringLiteral("Meta+Shift+M");
    }
    static QString autotileSwapMasterShortcut()
    {
        return QStringLiteral("Meta+Shift+Return");
    }
    static QString autotileIncMasterRatioShortcut()
    {
        return QStringLiteral("Meta+Shift+L");
    }
    static QString autotileDecMasterRatioShortcut()
    {
        return QStringLiteral("Meta+Shift+H");
    }
    static QString autotileIncMasterCountShortcut()
    {
        return QStringLiteral("Meta+Shift+]");
    }
    static QString autotileDecMasterCountShortcut()
    {
        return QStringLiteral("Meta+Shift+[");
    }
    static QString autotileRetileShortcut()
    {
        return QStringLiteral("Meta+Ctrl+R");
    }

private:
    // Non-instantiable
    ConfigDefaults() = delete;
};

} // namespace PlasmaZones
