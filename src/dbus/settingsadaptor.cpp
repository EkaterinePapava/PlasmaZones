// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingsadaptor.h"
#include "../core/interfaces.h"
#include "../config/settings.h" // For concrete Settings type
#include "../core/logging.h"
#include "../core/shaderregistry.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QColor>
#include <QDBusVariant>

namespace PlasmaZones {

SettingsAdaptor::SettingsAdaptor(ISettings* settings, QObject* parent)
    : QDBusAbstractAdaptor(parent)
    , m_settings(settings)
    , m_saveTimer(new QTimer(this))
{
    Q_ASSERT(settings);
    initializeRegistry();

    // Configure debounced save timer (performance optimization)
    m_saveTimer->setSingleShot(true);
    m_saveTimer->setInterval(SaveDebounceMs);
    connect(m_saveTimer, &QTimer::timeout, this, [this]() {
        m_settings->save();
        qCInfo(lcDbusSettings) << "Settings save completed";
    });

    // Connect to interface signals (DIP)
    connect(m_settings, &ISettings::settingsChanged, this, &SettingsAdaptor::settingsChanged);
}

SettingsAdaptor::~SettingsAdaptor()
{
    // Flush any pending debounced saves before destruction
    // This ensures settings are not lost on shutdown
    if (m_saveTimer->isActive()) {
        m_saveTimer->stop();
        m_settings->save();
        qCInfo(lcDbusSettings) << "Flushed pending save on destruction";
    }
}

void SettingsAdaptor::scheduleSave()
{
    // Restart the timer on each setting change (debouncing)
    // This batches multiple rapid changes into a single save
    m_saveTimer->start();
}

// Note: Macros are defined in initializeRegistry() to avoid namespace pollution

void SettingsAdaptor::initializeRegistry()
{
// Register getters and setters for all settings
// This registry pattern allows adding new settings without modifying setSetting()
// Using macros to reduce repetition

// Helper macros
#define REGISTER_STRING_SETTING(name, getter, setter)                                                                  \
    m_getters[QStringLiteral(name)] = [this]() {                                                                       \
        return m_settings->getter();                                                                                   \
    };                                                                                                                 \
    m_setters[QStringLiteral(name)] = [this](const QVariant& v) {                                                      \
        m_settings->setter(v.toString());                                                                              \
        return true;                                                                                                   \
    };

#define REGISTER_BOOL_SETTING(name, getter, setter)                                                                    \
    m_getters[QStringLiteral(name)] = [this]() {                                                                       \
        return m_settings->getter();                                                                                   \
    };                                                                                                                 \
    m_setters[QStringLiteral(name)] = [this](const QVariant& v) {                                                      \
        m_settings->setter(v.toBool());                                                                                \
        return true;                                                                                                   \
    };

#define REGISTER_INT_SETTING(name, getter, setter)                                                                     \
    m_getters[QStringLiteral(name)] = [this]() {                                                                       \
        return m_settings->getter();                                                                                   \
    };                                                                                                                 \
    m_setters[QStringLiteral(name)] = [this](const QVariant& v) {                                                      \
        m_settings->setter(v.toInt());                                                                                 \
        return true;                                                                                                   \
    };

#define REGISTER_DOUBLE_SETTING(name, getter, setter)                                                                  \
    m_getters[QStringLiteral(name)] = [this]() {                                                                       \
        return m_settings->getter();                                                                                   \
    };                                                                                                                 \
    m_setters[QStringLiteral(name)] = [this](const QVariant& v) {                                                      \
        m_settings->setter(v.toDouble());                                                                              \
        return true;                                                                                                   \
    };

#define REGISTER_COLOR_SETTING(keyName, getter, setter)                                                                \
    m_getters[QStringLiteral(keyName)] = [this]() {                                                                    \
        QColor color = m_settings->getter();                                                                           \
        return color.name(QColor::HexArgb);                                                                            \
    };                                                                                                                 \
    m_setters[QStringLiteral(keyName)] = [this](const QVariant& v) {                                                   \
        m_settings->setter(QColor(v.toString()));                                                                      \
        return true;                                                                                                   \
    };

#define REGISTER_STRINGLIST_SETTING(name, getter, setter)                                                              \
    m_getters[QStringLiteral(name)] = [this]() {                                                                       \
        return m_settings->getter();                                                                                   \
    };                                                                                                                 \
    m_setters[QStringLiteral(name)] = [this](const QVariant& v) {                                                      \
        m_settings->setter(v.toStringList());                                                                          \
        return true;                                                                                                   \
    };

    // Concrete Settings pointer for properties not on ISettings interface
    auto* concrete = qobject_cast<Settings*>(m_settings);

    // Activation settings
    REGISTER_BOOL_SETTING("shiftDragToActivate", shiftDragToActivate, setShiftDragToActivate)

    // Drag activation triggers list (multi-bind)
    m_getters[QStringLiteral("dragActivationTriggers")] = [this]() {
        return QVariant::fromValue(m_settings->dragActivationTriggers());
    };
    m_setters[QStringLiteral("dragActivationTriggers")] = [this](const QVariant& v) {
        m_settings->setDragActivationTriggers(v.toList());
        return true;
    };

    REGISTER_BOOL_SETTING("zoneSpanEnabled", zoneSpanEnabled, setZoneSpanEnabled)

    // Zone span modifier (legacy single value)
    m_getters[QStringLiteral("zoneSpanModifier")] = [this]() {
        return static_cast<int>(m_settings->zoneSpanModifier());
    };
    m_setters[QStringLiteral("zoneSpanModifier")] = [this](const QVariant& v) {
        int mod = v.toInt();
        if (mod >= 0 && mod <= static_cast<int>(DragModifier::CtrlAltMeta)) {
            m_settings->setZoneSpanModifier(static_cast<DragModifier>(mod));
            return true;
        }
        return false;
    };

    // Zone span triggers list (multi-bind)
    m_getters[QStringLiteral("zoneSpanTriggers")] = [this]() {
        return QVariant::fromValue(m_settings->zoneSpanTriggers());
    };
    m_setters[QStringLiteral("zoneSpanTriggers")] = [this](const QVariant& v) {
        m_settings->setZoneSpanTriggers(v.toList());
        return true;
    };

    REGISTER_BOOL_SETTING("toggleActivation", toggleActivation, setToggleActivation)
    REGISTER_BOOL_SETTING("snappingEnabled", snappingEnabled, setSnappingEnabled)

    // Display settings
    REGISTER_BOOL_SETTING("showZonesOnAllMonitors", showZonesOnAllMonitors, setShowZonesOnAllMonitors)
    REGISTER_BOOL_SETTING("showZoneNumbers", showZoneNumbers, setShowZoneNumbers)
    REGISTER_BOOL_SETTING("flashZonesOnSwitch", flashZonesOnSwitch, setFlashZonesOnSwitch)
    REGISTER_BOOL_SETTING("showOsdOnLayoutSwitch", showOsdOnLayoutSwitch, setShowOsdOnLayoutSwitch)
    REGISTER_BOOL_SETTING("showNavigationOsd", showNavigationOsd, setShowNavigationOsd)
    // osdStyle: enum (0=None, 1=Text, 2=Preview) — use interface's OsdStyle
    m_getters[QStringLiteral("osdStyle")] = [this]() {
        return static_cast<int>(m_settings->osdStyle());
    };
    m_setters[QStringLiteral("osdStyle")] = [this](const QVariant& v) {
        int val = v.toInt();
        if (val >= 0 && val <= 2) {
            m_settings->setOsdStyle(static_cast<OsdStyle>(val));
            return true;
        }
        return false;
    };
    // overlayDisplayMode: enum (0=ZoneRectangles, 1=LayoutPreview)
    m_getters[QStringLiteral("overlayDisplayMode")] = [this]() {
        return static_cast<int>(m_settings->overlayDisplayMode());
    };
    m_setters[QStringLiteral("overlayDisplayMode")] = [this](const QVariant& v) {
        int val = v.toInt();
        if (val >= 0 && val <= 1) {
            m_settings->setOverlayDisplayMode(static_cast<OverlayDisplayMode>(val));
            return true;
        }
        return false;
    };
    REGISTER_STRINGLIST_SETTING("disabledMonitors", disabledMonitors, setDisabledMonitors)

    // Appearance settings
    REGISTER_BOOL_SETTING("useSystemColors", useSystemColors, setUseSystemColors)
    REGISTER_COLOR_SETTING("highlightColor", highlightColor, setHighlightColor)
    REGISTER_COLOR_SETTING("inactiveColor", inactiveColor, setInactiveColor)
    REGISTER_COLOR_SETTING("borderColor", borderColor, setBorderColor)
    REGISTER_COLOR_SETTING("labelFontColor", labelFontColor, setLabelFontColor)
    REGISTER_DOUBLE_SETTING("activeOpacity", activeOpacity, setActiveOpacity)
    REGISTER_DOUBLE_SETTING("inactiveOpacity", inactiveOpacity, setInactiveOpacity)
    REGISTER_INT_SETTING("borderWidth", borderWidth, setBorderWidth)
    REGISTER_INT_SETTING("borderRadius", borderRadius, setBorderRadius)
    REGISTER_BOOL_SETTING("enableBlur", enableBlur, setEnableBlur)
    REGISTER_STRING_SETTING("labelFontFamily", labelFontFamily, setLabelFontFamily)
    // Custom setter with range validation (0.25-3.0) instead of REGISTER_DOUBLE_SETTING
    m_getters[QStringLiteral("labelFontSizeScale")] = [this]() {
        return m_settings->labelFontSizeScale();
    };
    m_setters[QStringLiteral("labelFontSizeScale")] = [this](const QVariant& v) {
        bool ok;
        double val = v.toDouble(&ok);
        if (!ok || val < 0.25 || val > 3.0) {
            return false;
        }
        m_settings->setLabelFontSizeScale(val);
        return true;
    };
    REGISTER_INT_SETTING("labelFontWeight", labelFontWeight, setLabelFontWeight)
    REGISTER_BOOL_SETTING("labelFontItalic", labelFontItalic, setLabelFontItalic)
    REGISTER_BOOL_SETTING("labelFontUnderline", labelFontUnderline, setLabelFontUnderline)
    REGISTER_BOOL_SETTING("labelFontStrikeout", labelFontStrikeout, setLabelFontStrikeout)
    REGISTER_BOOL_SETTING("enableShaderEffects", enableShaderEffects, setEnableShaderEffects)
    REGISTER_INT_SETTING("shaderFrameRate", shaderFrameRate, setShaderFrameRate)
    REGISTER_BOOL_SETTING("enableAudioVisualizer", enableAudioVisualizer, setEnableAudioVisualizer)
    REGISTER_INT_SETTING("audioSpectrumBarCount", audioSpectrumBarCount, setAudioSpectrumBarCount)

    // Zone settings
    REGISTER_INT_SETTING("zonePadding", zonePadding, setZonePadding)
    REGISTER_INT_SETTING("outerGap", outerGap, setOuterGap)
    REGISTER_BOOL_SETTING("usePerSideOuterGap", usePerSideOuterGap, setUsePerSideOuterGap)
    REGISTER_INT_SETTING("outerGapTop", outerGapTop, setOuterGapTop)
    REGISTER_INT_SETTING("outerGapBottom", outerGapBottom, setOuterGapBottom)
    REGISTER_INT_SETTING("outerGapLeft", outerGapLeft, setOuterGapLeft)
    REGISTER_INT_SETTING("outerGapRight", outerGapRight, setOuterGapRight)
    REGISTER_INT_SETTING("adjacentThreshold", adjacentThreshold, setAdjacentThreshold)
    REGISTER_INT_SETTING("pollIntervalMs", pollIntervalMs, setPollIntervalMs)
    REGISTER_INT_SETTING("minimumZoneSizePx", minimumZoneSizePx, setMinimumZoneSizePx)
    REGISTER_INT_SETTING("minimumZoneDisplaySizePx", minimumZoneDisplaySizePx, setMinimumZoneDisplaySizePx)

    // Behavior settings
    REGISTER_BOOL_SETTING("keepWindowsInZonesOnResolutionChange", keepWindowsInZonesOnResolutionChange,
                          setKeepWindowsInZonesOnResolutionChange)
    REGISTER_BOOL_SETTING("moveNewWindowsToLastZone", moveNewWindowsToLastZone, setMoveNewWindowsToLastZone)
    REGISTER_BOOL_SETTING("restoreOriginalSizeOnUnsnap", restoreOriginalSizeOnUnsnap, setRestoreOriginalSizeOnUnsnap)
    // stickyWindowHandling: enum (0=TreatAsNormal, 1=RestoreOnly, 2=IgnoreAll)
    m_getters[QStringLiteral("stickyWindowHandling")] = [this]() {
        return static_cast<int>(m_settings->stickyWindowHandling());
    };
    m_setters[QStringLiteral("stickyWindowHandling")] = [this](const QVariant& v) {
        int val = v.toInt();
        if (val >= 0 && val <= 2) {
            m_settings->setStickyWindowHandling(static_cast<StickyWindowHandling>(val));
            return true;
        }
        return false;
    };
    REGISTER_BOOL_SETTING("restoreWindowsToZonesOnLogin", restoreWindowsToZonesOnLogin, setRestoreWindowsToZonesOnLogin)
    REGISTER_BOOL_SETTING("snapAssistFeatureEnabled", snapAssistFeatureEnabled, setSnapAssistFeatureEnabled)
    REGISTER_BOOL_SETTING("snapAssistEnabled", snapAssistEnabled, setSnapAssistEnabled)

    // Snap assist triggers (when always-enabled is off, hold any trigger at drop to enable)
    m_getters[QStringLiteral("snapAssistTriggers")] = [this]() {
        return QVariant::fromValue(m_settings->snapAssistTriggers());
    };
    m_setters[QStringLiteral("snapAssistTriggers")] = [this](const QVariant& v) {
        m_settings->setSnapAssistTriggers(v.toList());
        return true;
    };

    // Default layout
    REGISTER_STRING_SETTING("defaultLayoutId", defaultLayoutId, setDefaultLayoutId)

    // Exclusions
    REGISTER_STRINGLIST_SETTING("excludedApplications", excludedApplications, setExcludedApplications)
    REGISTER_STRINGLIST_SETTING("excludedWindowClasses", excludedWindowClasses, setExcludedWindowClasses)
    REGISTER_BOOL_SETTING("excludeTransientWindows", excludeTransientWindows, setExcludeTransientWindows)
    REGISTER_INT_SETTING("minimumWindowWidth", minimumWindowWidth, setMinimumWindowWidth)
    REGISTER_INT_SETTING("minimumWindowHeight", minimumWindowHeight, setMinimumWindowHeight)

    // Zone selector settings
    REGISTER_BOOL_SETTING("zoneSelectorEnabled", zoneSelectorEnabled, setZoneSelectorEnabled)
    REGISTER_INT_SETTING("zoneSelectorTriggerDistance", zoneSelectorTriggerDistance, setZoneSelectorTriggerDistance)
    // zoneSelectorPosition: enum (0=TopLeft .. 8=BottomRight)
    m_getters[QStringLiteral("zoneSelectorPosition")] = [this]() {
        return static_cast<int>(m_settings->zoneSelectorPosition());
    };
    m_setters[QStringLiteral("zoneSelectorPosition")] = [this](const QVariant& v) {
        int val = v.toInt();
        if (val >= 0 && val <= 8) {
            m_settings->setZoneSelectorPosition(static_cast<ZoneSelectorPosition>(val));
            return true;
        }
        return false;
    };
    // zoneSelectorLayoutMode: enum (0=Grid, 1=Horizontal, 2=Vertical)
    m_getters[QStringLiteral("zoneSelectorLayoutMode")] = [this]() {
        return static_cast<int>(m_settings->zoneSelectorLayoutMode());
    };
    m_setters[QStringLiteral("zoneSelectorLayoutMode")] = [this](const QVariant& v) {
        int val = v.toInt();
        if (val >= 0 && val <= 2) {
            m_settings->setZoneSelectorLayoutMode(static_cast<ZoneSelectorLayoutMode>(val));
            return true;
        }
        return false;
    };
    // zoneSelectorSizeMode: enum (0=Auto, 1=Manual)
    m_getters[QStringLiteral("zoneSelectorSizeMode")] = [this]() {
        return static_cast<int>(m_settings->zoneSelectorSizeMode());
    };
    m_setters[QStringLiteral("zoneSelectorSizeMode")] = [this](const QVariant& v) {
        int val = v.toInt();
        if (val >= 0 && val <= 1) {
            m_settings->setZoneSelectorSizeMode(static_cast<ZoneSelectorSizeMode>(val));
            return true;
        }
        return false;
    };
    REGISTER_INT_SETTING("zoneSelectorMaxRows", zoneSelectorMaxRows, setZoneSelectorMaxRows)
    REGISTER_INT_SETTING("zoneSelectorPreviewWidth", zoneSelectorPreviewWidth, setZoneSelectorPreviewWidth)
    REGISTER_INT_SETTING("zoneSelectorPreviewHeight", zoneSelectorPreviewHeight, setZoneSelectorPreviewHeight)
    REGISTER_BOOL_SETTING("zoneSelectorPreviewLockAspect", zoneSelectorPreviewLockAspect,
                          setZoneSelectorPreviewLockAspect)
    REGISTER_INT_SETTING("zoneSelectorGridColumns", zoneSelectorGridColumns, setZoneSelectorGridColumns)

    // Animation settings (global — applies to snapping and autotiling)
    REGISTER_BOOL_SETTING("animationsEnabled", animationsEnabled, setAnimationsEnabled)
    REGISTER_INT_SETTING("animationDuration", animationDuration, setAnimationDuration)
    REGISTER_STRING_SETTING("animationEasingCurve", animationEasingCurve, setAnimationEasingCurve)
    REGISTER_INT_SETTING("animationMinDistance", animationMinDistance, setAnimationMinDistance)
    REGISTER_INT_SETTING("animationSequenceMode", animationSequenceMode, setAnimationSequenceMode)
    REGISTER_INT_SETTING("animationStaggerInterval", animationStaggerInterval, setAnimationStaggerInterval)

    // Autotile core settings (concrete Settings only)
    if (concrete) {
        m_getters[QStringLiteral("autotileEnabled")] = [concrete]() {
            return concrete->autotileEnabled();
        };
        m_setters[QStringLiteral("autotileEnabled")] = [concrete](const QVariant& v) {
            concrete->setAutotileEnabled(v.toBool());
            return true;
        };
        m_getters[QStringLiteral("autotileAlgorithm")] = [concrete]() {
            return concrete->autotileAlgorithm();
        };
        m_setters[QStringLiteral("autotileAlgorithm")] = [concrete](const QVariant& v) {
            concrete->setAutotileAlgorithm(v.toString());
            return true;
        };
        m_getters[QStringLiteral("autotileSplitRatio")] = [concrete]() {
            return concrete->autotileSplitRatio();
        };
        m_setters[QStringLiteral("autotileSplitRatio")] = [concrete](const QVariant& v) {
            concrete->setAutotileSplitRatio(v.toDouble());
            return true;
        };
        m_getters[QStringLiteral("autotileMasterCount")] = [concrete]() {
            return concrete->autotileMasterCount();
        };
        m_setters[QStringLiteral("autotileMasterCount")] = [concrete](const QVariant& v) {
            concrete->setAutotileMasterCount(v.toInt());
            return true;
        };
        m_getters[QStringLiteral("autotileCenteredMasterSplitRatio")] = [concrete]() {
            return concrete->autotileCenteredMasterSplitRatio();
        };
        m_setters[QStringLiteral("autotileCenteredMasterSplitRatio")] = [concrete](const QVariant& v) {
            concrete->setAutotileCenteredMasterSplitRatio(v.toDouble());
            return true;
        };
        m_getters[QStringLiteral("autotileCenteredMasterMasterCount")] = [concrete]() {
            return concrete->autotileCenteredMasterMasterCount();
        };
        m_setters[QStringLiteral("autotileCenteredMasterMasterCount")] = [concrete](const QVariant& v) {
            concrete->setAutotileCenteredMasterMasterCount(v.toInt());
            return true;
        };
        m_getters[QStringLiteral("autotileInnerGap")] = [concrete]() {
            return concrete->autotileInnerGap();
        };
        m_setters[QStringLiteral("autotileInnerGap")] = [concrete](const QVariant& v) {
            concrete->setAutotileInnerGap(v.toInt());
            return true;
        };
        m_getters[QStringLiteral("autotileOuterGap")] = [concrete]() {
            return concrete->autotileOuterGap();
        };
        m_setters[QStringLiteral("autotileOuterGap")] = [concrete](const QVariant& v) {
            concrete->setAutotileOuterGap(v.toInt());
            return true;
        };
        m_getters[QStringLiteral("autotileUsePerSideOuterGap")] = [concrete]() {
            return concrete->autotileUsePerSideOuterGap();
        };
        m_setters[QStringLiteral("autotileUsePerSideOuterGap")] = [concrete](const QVariant& v) {
            concrete->setAutotileUsePerSideOuterGap(v.toBool());
            return true;
        };
        m_getters[QStringLiteral("autotileOuterGapTop")] = [concrete]() {
            return concrete->autotileOuterGapTop();
        };
        m_setters[QStringLiteral("autotileOuterGapTop")] = [concrete](const QVariant& v) {
            concrete->setAutotileOuterGapTop(v.toInt());
            return true;
        };
        m_getters[QStringLiteral("autotileOuterGapBottom")] = [concrete]() {
            return concrete->autotileOuterGapBottom();
        };
        m_setters[QStringLiteral("autotileOuterGapBottom")] = [concrete](const QVariant& v) {
            concrete->setAutotileOuterGapBottom(v.toInt());
            return true;
        };
        m_getters[QStringLiteral("autotileOuterGapLeft")] = [concrete]() {
            return concrete->autotileOuterGapLeft();
        };
        m_setters[QStringLiteral("autotileOuterGapLeft")] = [concrete](const QVariant& v) {
            concrete->setAutotileOuterGapLeft(v.toInt());
            return true;
        };
        m_getters[QStringLiteral("autotileOuterGapRight")] = [concrete]() {
            return concrete->autotileOuterGapRight();
        };
        m_setters[QStringLiteral("autotileOuterGapRight")] = [concrete](const QVariant& v) {
            concrete->setAutotileOuterGapRight(v.toInt());
            return true;
        };
        m_getters[QStringLiteral("autotileFocusNewWindows")] = [concrete]() {
            return concrete->autotileFocusNewWindows();
        };
        m_setters[QStringLiteral("autotileFocusNewWindows")] = [concrete](const QVariant& v) {
            concrete->setAutotileFocusNewWindows(v.toBool());
            return true;
        };
        m_getters[QStringLiteral("autotileSmartGaps")] = [concrete]() {
            return concrete->autotileSmartGaps();
        };
        m_setters[QStringLiteral("autotileSmartGaps")] = [concrete](const QVariant& v) {
            concrete->setAutotileSmartGaps(v.toBool());
            return true;
        };
        m_getters[QStringLiteral("autotileMaxWindows")] = [concrete]() {
            return concrete->autotileMaxWindows();
        };
        m_setters[QStringLiteral("autotileMaxWindows")] = [concrete](const QVariant& v) {
            concrete->setAutotileMaxWindows(v.toInt());
            return true;
        };
        // autotileInsertPosition: enum (0=End, 1=AfterFocused, 2=AsMaster)
        m_getters[QStringLiteral("autotileInsertPosition")] = [concrete]() {
            return static_cast<int>(concrete->autotileInsertPosition());
        };
        m_setters[QStringLiteral("autotileInsertPosition")] = [concrete](const QVariant& v) {
            int val = v.toInt();
            if (val >= 0 && val <= 2) {
                concrete->setAutotileInsertPosition(static_cast<Settings::AutotileInsertPosition>(val));
                return true;
            }
            return false;
        };
        m_getters[QStringLiteral("autotileRespectMinimumSize")] = [concrete]() {
            return concrete->autotileRespectMinimumSize();
        };
        m_setters[QStringLiteral("autotileRespectMinimumSize")] = [concrete](const QVariant& v) {
            concrete->setAutotileRespectMinimumSize(v.toBool());
            return true;
        };
    }

    // Autotile decoration settings (on ISettings interface)
    REGISTER_BOOL_SETTING("autotileHideTitleBars", autotileHideTitleBars, setAutotileHideTitleBars)
    REGISTER_BOOL_SETTING("autotileShowBorder", autotileShowBorder, setAutotileShowBorder)
    REGISTER_INT_SETTING("autotileBorderWidth", autotileBorderWidth, setAutotileBorderWidth)
    REGISTER_INT_SETTING("autotileBorderRadius", autotileBorderRadius, setAutotileBorderRadius)
    REGISTER_COLOR_SETTING("autotileBorderColor", autotileBorderColor, setAutotileBorderColor)
    REGISTER_COLOR_SETTING("autotileInactiveBorderColor", autotileInactiveBorderColor, setAutotileInactiveBorderColor)
    REGISTER_BOOL_SETTING("autotileUseSystemBorderColors", autotileUseSystemBorderColors,
                          setAutotileUseSystemBorderColors)
    REGISTER_BOOL_SETTING("autotileFocusFollowsMouse", autotileFocusFollowsMouse, setAutotileFocusFollowsMouse)
    REGISTER_STRINGLIST_SETTING("lockedScreens", lockedScreens, setLockedScreens)

    // Autotile shortcuts (concrete Settings only)
    if (concrete) {
        m_getters[QStringLiteral("autotileToggleShortcut")] = [concrete]() {
            return concrete->autotileToggleShortcut();
        };
        m_setters[QStringLiteral("autotileToggleShortcut")] = [concrete](const QVariant& v) {
            concrete->setAutotileToggleShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("autotileFocusMasterShortcut")] = [concrete]() {
            return concrete->autotileFocusMasterShortcut();
        };
        m_setters[QStringLiteral("autotileFocusMasterShortcut")] = [concrete](const QVariant& v) {
            concrete->setAutotileFocusMasterShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("autotileSwapMasterShortcut")] = [concrete]() {
            return concrete->autotileSwapMasterShortcut();
        };
        m_setters[QStringLiteral("autotileSwapMasterShortcut")] = [concrete](const QVariant& v) {
            concrete->setAutotileSwapMasterShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("autotileIncMasterRatioShortcut")] = [concrete]() {
            return concrete->autotileIncMasterRatioShortcut();
        };
        m_setters[QStringLiteral("autotileIncMasterRatioShortcut")] = [concrete](const QVariant& v) {
            concrete->setAutotileIncMasterRatioShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("autotileDecMasterRatioShortcut")] = [concrete]() {
            return concrete->autotileDecMasterRatioShortcut();
        };
        m_setters[QStringLiteral("autotileDecMasterRatioShortcut")] = [concrete](const QVariant& v) {
            concrete->setAutotileDecMasterRatioShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("autotileIncMasterCountShortcut")] = [concrete]() {
            return concrete->autotileIncMasterCountShortcut();
        };
        m_setters[QStringLiteral("autotileIncMasterCountShortcut")] = [concrete](const QVariant& v) {
            concrete->setAutotileIncMasterCountShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("autotileDecMasterCountShortcut")] = [concrete]() {
            return concrete->autotileDecMasterCountShortcut();
        };
        m_setters[QStringLiteral("autotileDecMasterCountShortcut")] = [concrete](const QVariant& v) {
            concrete->setAutotileDecMasterCountShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("autotileRetileShortcut")] = [concrete]() {
            return concrete->autotileRetileShortcut();
        };
        m_setters[QStringLiteral("autotileRetileShortcut")] = [concrete](const QVariant& v) {
            concrete->setAutotileRetileShortcut(v.toString());
            return true;
        };
    }

    // Global shortcuts (concrete Settings only)
    if (concrete) {
        m_getters[QStringLiteral("openEditorShortcut")] = [concrete]() {
            return concrete->openEditorShortcut();
        };
        m_setters[QStringLiteral("openEditorShortcut")] = [concrete](const QVariant& v) {
            concrete->setOpenEditorShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("previousLayoutShortcut")] = [concrete]() {
            return concrete->previousLayoutShortcut();
        };
        m_setters[QStringLiteral("previousLayoutShortcut")] = [concrete](const QVariant& v) {
            concrete->setPreviousLayoutShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("nextLayoutShortcut")] = [concrete]() {
            return concrete->nextLayoutShortcut();
        };
        m_setters[QStringLiteral("nextLayoutShortcut")] = [concrete](const QVariant& v) {
            concrete->setNextLayoutShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("quickLayout1Shortcut")] = [concrete]() {
            return concrete->quickLayout1Shortcut();
        };
        m_setters[QStringLiteral("quickLayout1Shortcut")] = [concrete](const QVariant& v) {
            concrete->setQuickLayout1Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("quickLayout2Shortcut")] = [concrete]() {
            return concrete->quickLayout2Shortcut();
        };
        m_setters[QStringLiteral("quickLayout2Shortcut")] = [concrete](const QVariant& v) {
            concrete->setQuickLayout2Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("quickLayout3Shortcut")] = [concrete]() {
            return concrete->quickLayout3Shortcut();
        };
        m_setters[QStringLiteral("quickLayout3Shortcut")] = [concrete](const QVariant& v) {
            concrete->setQuickLayout3Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("quickLayout4Shortcut")] = [concrete]() {
            return concrete->quickLayout4Shortcut();
        };
        m_setters[QStringLiteral("quickLayout4Shortcut")] = [concrete](const QVariant& v) {
            concrete->setQuickLayout4Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("quickLayout5Shortcut")] = [concrete]() {
            return concrete->quickLayout5Shortcut();
        };
        m_setters[QStringLiteral("quickLayout5Shortcut")] = [concrete](const QVariant& v) {
            concrete->setQuickLayout5Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("quickLayout6Shortcut")] = [concrete]() {
            return concrete->quickLayout6Shortcut();
        };
        m_setters[QStringLiteral("quickLayout6Shortcut")] = [concrete](const QVariant& v) {
            concrete->setQuickLayout6Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("quickLayout7Shortcut")] = [concrete]() {
            return concrete->quickLayout7Shortcut();
        };
        m_setters[QStringLiteral("quickLayout7Shortcut")] = [concrete](const QVariant& v) {
            concrete->setQuickLayout7Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("quickLayout8Shortcut")] = [concrete]() {
            return concrete->quickLayout8Shortcut();
        };
        m_setters[QStringLiteral("quickLayout8Shortcut")] = [concrete](const QVariant& v) {
            concrete->setQuickLayout8Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("quickLayout9Shortcut")] = [concrete]() {
            return concrete->quickLayout9Shortcut();
        };
        m_setters[QStringLiteral("quickLayout9Shortcut")] = [concrete](const QVariant& v) {
            concrete->setQuickLayout9Shortcut(v.toString());
            return true;
        };

        // Keyboard navigation shortcuts
        m_getters[QStringLiteral("moveWindowLeftShortcut")] = [concrete]() {
            return concrete->moveWindowLeftShortcut();
        };
        m_setters[QStringLiteral("moveWindowLeftShortcut")] = [concrete](const QVariant& v) {
            concrete->setMoveWindowLeftShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("moveWindowRightShortcut")] = [concrete]() {
            return concrete->moveWindowRightShortcut();
        };
        m_setters[QStringLiteral("moveWindowRightShortcut")] = [concrete](const QVariant& v) {
            concrete->setMoveWindowRightShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("moveWindowUpShortcut")] = [concrete]() {
            return concrete->moveWindowUpShortcut();
        };
        m_setters[QStringLiteral("moveWindowUpShortcut")] = [concrete](const QVariant& v) {
            concrete->setMoveWindowUpShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("moveWindowDownShortcut")] = [concrete]() {
            return concrete->moveWindowDownShortcut();
        };
        m_setters[QStringLiteral("moveWindowDownShortcut")] = [concrete](const QVariant& v) {
            concrete->setMoveWindowDownShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("focusZoneLeftShortcut")] = [concrete]() {
            return concrete->focusZoneLeftShortcut();
        };
        m_setters[QStringLiteral("focusZoneLeftShortcut")] = [concrete](const QVariant& v) {
            concrete->setFocusZoneLeftShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("focusZoneRightShortcut")] = [concrete]() {
            return concrete->focusZoneRightShortcut();
        };
        m_setters[QStringLiteral("focusZoneRightShortcut")] = [concrete](const QVariant& v) {
            concrete->setFocusZoneRightShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("focusZoneUpShortcut")] = [concrete]() {
            return concrete->focusZoneUpShortcut();
        };
        m_setters[QStringLiteral("focusZoneUpShortcut")] = [concrete](const QVariant& v) {
            concrete->setFocusZoneUpShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("focusZoneDownShortcut")] = [concrete]() {
            return concrete->focusZoneDownShortcut();
        };
        m_setters[QStringLiteral("focusZoneDownShortcut")] = [concrete](const QVariant& v) {
            concrete->setFocusZoneDownShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("pushToEmptyZoneShortcut")] = [concrete]() {
            return concrete->pushToEmptyZoneShortcut();
        };
        m_setters[QStringLiteral("pushToEmptyZoneShortcut")] = [concrete](const QVariant& v) {
            concrete->setPushToEmptyZoneShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("restoreWindowSizeShortcut")] = [concrete]() {
            return concrete->restoreWindowSizeShortcut();
        };
        m_setters[QStringLiteral("restoreWindowSizeShortcut")] = [concrete](const QVariant& v) {
            concrete->setRestoreWindowSizeShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("toggleWindowFloatShortcut")] = [concrete]() {
            return concrete->toggleWindowFloatShortcut();
        };
        m_setters[QStringLiteral("toggleWindowFloatShortcut")] = [concrete](const QVariant& v) {
            concrete->setToggleWindowFloatShortcut(v.toString());
            return true;
        };

        // Swap window shortcuts
        m_getters[QStringLiteral("swapWindowLeftShortcut")] = [concrete]() {
            return concrete->swapWindowLeftShortcut();
        };
        m_setters[QStringLiteral("swapWindowLeftShortcut")] = [concrete](const QVariant& v) {
            concrete->setSwapWindowLeftShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("swapWindowRightShortcut")] = [concrete]() {
            return concrete->swapWindowRightShortcut();
        };
        m_setters[QStringLiteral("swapWindowRightShortcut")] = [concrete](const QVariant& v) {
            concrete->setSwapWindowRightShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("swapWindowUpShortcut")] = [concrete]() {
            return concrete->swapWindowUpShortcut();
        };
        m_setters[QStringLiteral("swapWindowUpShortcut")] = [concrete](const QVariant& v) {
            concrete->setSwapWindowUpShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("swapWindowDownShortcut")] = [concrete]() {
            return concrete->swapWindowDownShortcut();
        };
        m_setters[QStringLiteral("swapWindowDownShortcut")] = [concrete](const QVariant& v) {
            concrete->setSwapWindowDownShortcut(v.toString());
            return true;
        };

        // Snap to zone by number shortcuts
        m_getters[QStringLiteral("snapToZone1Shortcut")] = [concrete]() {
            return concrete->snapToZone1Shortcut();
        };
        m_setters[QStringLiteral("snapToZone1Shortcut")] = [concrete](const QVariant& v) {
            concrete->setSnapToZone1Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("snapToZone2Shortcut")] = [concrete]() {
            return concrete->snapToZone2Shortcut();
        };
        m_setters[QStringLiteral("snapToZone2Shortcut")] = [concrete](const QVariant& v) {
            concrete->setSnapToZone2Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("snapToZone3Shortcut")] = [concrete]() {
            return concrete->snapToZone3Shortcut();
        };
        m_setters[QStringLiteral("snapToZone3Shortcut")] = [concrete](const QVariant& v) {
            concrete->setSnapToZone3Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("snapToZone4Shortcut")] = [concrete]() {
            return concrete->snapToZone4Shortcut();
        };
        m_setters[QStringLiteral("snapToZone4Shortcut")] = [concrete](const QVariant& v) {
            concrete->setSnapToZone4Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("snapToZone5Shortcut")] = [concrete]() {
            return concrete->snapToZone5Shortcut();
        };
        m_setters[QStringLiteral("snapToZone5Shortcut")] = [concrete](const QVariant& v) {
            concrete->setSnapToZone5Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("snapToZone6Shortcut")] = [concrete]() {
            return concrete->snapToZone6Shortcut();
        };
        m_setters[QStringLiteral("snapToZone6Shortcut")] = [concrete](const QVariant& v) {
            concrete->setSnapToZone6Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("snapToZone7Shortcut")] = [concrete]() {
            return concrete->snapToZone7Shortcut();
        };
        m_setters[QStringLiteral("snapToZone7Shortcut")] = [concrete](const QVariant& v) {
            concrete->setSnapToZone7Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("snapToZone8Shortcut")] = [concrete]() {
            return concrete->snapToZone8Shortcut();
        };
        m_setters[QStringLiteral("snapToZone8Shortcut")] = [concrete](const QVariant& v) {
            concrete->setSnapToZone8Shortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("snapToZone9Shortcut")] = [concrete]() {
            return concrete->snapToZone9Shortcut();
        };
        m_setters[QStringLiteral("snapToZone9Shortcut")] = [concrete](const QVariant& v) {
            concrete->setSnapToZone9Shortcut(v.toString());
            return true;
        };

        // Rotate windows shortcuts
        m_getters[QStringLiteral("rotateWindowsClockwiseShortcut")] = [concrete]() {
            return concrete->rotateWindowsClockwiseShortcut();
        };
        m_setters[QStringLiteral("rotateWindowsClockwiseShortcut")] = [concrete](const QVariant& v) {
            concrete->setRotateWindowsClockwiseShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("rotateWindowsCounterclockwiseShortcut")] = [concrete]() {
            return concrete->rotateWindowsCounterclockwiseShortcut();
        };
        m_setters[QStringLiteral("rotateWindowsCounterclockwiseShortcut")] = [concrete](const QVariant& v) {
            concrete->setRotateWindowsCounterclockwiseShortcut(v.toString());
            return true;
        };

        // Cycle windows in zone shortcuts
        m_getters[QStringLiteral("cycleWindowForwardShortcut")] = [concrete]() {
            return concrete->cycleWindowForwardShortcut();
        };
        m_setters[QStringLiteral("cycleWindowForwardShortcut")] = [concrete](const QVariant& v) {
            concrete->setCycleWindowForwardShortcut(v.toString());
            return true;
        };
        m_getters[QStringLiteral("cycleWindowBackwardShortcut")] = [concrete]() {
            return concrete->cycleWindowBackwardShortcut();
        };
        m_setters[QStringLiteral("cycleWindowBackwardShortcut")] = [concrete](const QVariant& v) {
            concrete->setCycleWindowBackwardShortcut(v.toString());
            return true;
        };

        // Resnap to new layout shortcut
        m_getters[QStringLiteral("resnapToNewLayoutShortcut")] = [concrete]() {
            return concrete->resnapToNewLayoutShortcut();
        };
        m_setters[QStringLiteral("resnapToNewLayoutShortcut")] = [concrete](const QVariant& v) {
            concrete->setResnapToNewLayoutShortcut(v.toString());
            return true;
        };

        // Snap all windows shortcut
        m_getters[QStringLiteral("snapAllWindowsShortcut")] = [concrete]() {
            return concrete->snapAllWindowsShortcut();
        };
        m_setters[QStringLiteral("snapAllWindowsShortcut")] = [concrete](const QVariant& v) {
            concrete->setSnapAllWindowsShortcut(v.toString());
            return true;
        };

        // Layout picker shortcut
        m_getters[QStringLiteral("layoutPickerShortcut")] = [concrete]() {
            return concrete->layoutPickerShortcut();
        };
        m_setters[QStringLiteral("layoutPickerShortcut")] = [concrete](const QVariant& v) {
            concrete->setLayoutPickerShortcut(v.toString());
            return true;
        };

        // Toggle layout lock shortcut
        m_getters[QStringLiteral("toggleLayoutLockShortcut")] = [concrete]() {
            return concrete->toggleLayoutLockShortcut();
        };
        m_setters[QStringLiteral("toggleLayoutLockShortcut")] = [concrete](const QVariant& v) {
            concrete->setToggleLayoutLockShortcut(v.toString());
            return true;
        };
    }

// Clean up macros (local scope)
#undef REGISTER_STRING_SETTING
#undef REGISTER_BOOL_SETTING
#undef REGISTER_INT_SETTING
#undef REGISTER_DOUBLE_SETTING
#undef REGISTER_COLOR_SETTING
#undef REGISTER_STRINGLIST_SETTING
}

void SettingsAdaptor::reloadSettings()
{
    m_settings->load();
}

void SettingsAdaptor::saveSettings()
{
    m_settings->save();
}

void SettingsAdaptor::resetToDefaults()
{
    m_settings->reset();
}

QString SettingsAdaptor::getAllSettings()
{
    QJsonObject settings;
    for (auto it = m_getters.constBegin(); it != m_getters.constEnd(); ++it) {
        settings[it.key()] = QJsonValue::fromVariant(it.value()());
    }
    return QString::fromUtf8(QJsonDocument(settings).toJson());
}

QDBusVariant SettingsAdaptor::getSetting(const QString& key)
{
    if (key.isEmpty()) {
        qCWarning(lcDbusSettings) << "getSetting: empty key";
        // Return a valid but empty QDBusVariant to avoid marshalling errors
        // (QDBusVariant() with no argument creates an invalid variant that can't be sent)
        return QDBusVariant(QVariant(QString()));
    }

    auto it = m_getters.find(key);
    if (it != m_getters.end()) {
        QVariant value = it.value()();
        // Ensure we never return an invalid variant - use empty string as fallback
        if (!value.isValid()) {
            qCWarning(lcDbusSettings) << "Setting" << key << "returned invalid variant, using empty string";
            return QDBusVariant(QVariant(QString()));
        }
        return QDBusVariant(value);
    }

    qCWarning(lcDbusSettings) << "Setting key not found:" << key;
    // Return a valid but empty QDBusVariant with error indicator
    // Callers should check for empty string as "not found" indicator
    return QDBusVariant(QVariant(QString()));
}

bool SettingsAdaptor::setSetting(const QString& key, const QDBusVariant& value)
{
    if (key.isEmpty()) {
        qCWarning(lcDbusSettings) << "setSetting: empty key";
        return false;
    }

    auto it = m_setters.find(key);
    if (it != m_setters.end()) {
        bool result = it.value()(value.variant());
        if (result) {
            // Use debounced save instead of immediate save (performance optimization)
            // This batches multiple rapid setting changes into a single disk write
            scheduleSave();
            qCInfo(lcDbusSettings) << "Setting" << key << "updated, save scheduled";
        } else {
            qCWarning(lcDbusSettings) << "Failed to set setting:" << key;
        }
        return result;
    }

    qCWarning(lcDbusSettings) << "Setting key not found:" << key;
    return false;
}

bool SettingsAdaptor::setSettings(const QVariantMap& settings)
{
    if (settings.isEmpty()) {
        qCWarning(lcDbusSettings) << "setSettings: empty map";
        return false;
    }

    // Stop any pending debounced save — we will save synchronously below
    m_saveTimer->stop();

    bool allOk = true;
    for (auto it = settings.constBegin(); it != settings.constEnd(); ++it) {
        const QString& key = it.key();
        auto setter = m_setters.find(key);
        if (setter == m_setters.end()) {
            qCWarning(lcDbusSettings) << "setSettings: unknown key" << key;
            allOk = false;
            continue;
        }
        if (!setter.value()(it.value())) {
            qCWarning(lcDbusSettings) << "setSettings: setter failed for key" << key;
            allOk = false;
        }
    }

    // Single synchronous save for the entire batch
    m_settings->save();
    qCInfo(lcDbusSettings) << "setSettings: batch applied" << settings.size() << "keys, allOk:" << allOk;

    return allOk;
}

QStringList SettingsAdaptor::getSettingKeys()
{
    return m_getters.keys();
}

// ═══════════════════════════════════════════════════════════════════════════════
// Shader Registry D-Bus Methods
// ═══════════════════════════════════════════════════════════════════════════════

QVariantList SettingsAdaptor::availableShaders()
{
    auto* registry = ShaderRegistry::instance();
    return registry ? registry->availableShadersVariant() : QVariantList();
}

QVariantMap SettingsAdaptor::shaderInfo(const QString& shaderId)
{
    auto* registry = ShaderRegistry::instance();
    return registry ? registry->shaderInfo(shaderId) : QVariantMap();
}

QVariantMap SettingsAdaptor::defaultShaderParams(const QString& shaderId)
{
    auto* registry = ShaderRegistry::instance();
    return registry ? registry->defaultParams(shaderId) : QVariantMap();
}

QVariantMap SettingsAdaptor::translateShaderParams(const QString& shaderId, const QVariantMap& params)
{
    auto* registry = ShaderRegistry::instance();
    return registry ? registry->translateParamsToUniforms(shaderId, params) : QVariantMap();
}

bool SettingsAdaptor::shadersEnabled()
{
    auto* registry = ShaderRegistry::instance();
    return registry ? registry->shadersEnabled() : false;
}

bool SettingsAdaptor::userShadersEnabled()
{
    auto* registry = ShaderRegistry::instance();
    return registry ? registry->userShadersEnabled() : false;
}

QString SettingsAdaptor::userShaderDirectory()
{
    auto* registry = ShaderRegistry::instance();
    return registry ? registry->userShaderDirectory() : QString();
}

void SettingsAdaptor::openUserShaderDirectory()
{
    auto* registry = ShaderRegistry::instance();
    if (registry) {
        registry->openUserShaderDirectory();
    }
}

void SettingsAdaptor::refreshShaders()
{
    auto* registry = ShaderRegistry::instance();
    if (registry) {
        registry->refresh();
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Window Picker D-Bus Methods
// ═══════════════════════════════════════════════════════════════════════════════

QString SettingsAdaptor::getRunningWindows()
{
    // Guard against reentrant calls (shouldn't happen via D-Bus serialization,
    // but protects against unexpected provideRunningWindows calls)
    if (m_windowListLoop) {
        return QStringLiteral("[]");
    }

    m_pendingWindowList.clear();

    QEventLoop loop;
    m_windowListLoop = &loop;

    // Blocking call: waits for KWin effect to respond via provideRunningWindows().
    // The 2s timeout prevents indefinite blocking if the effect is unloaded or
    // unresponsive. This is called from the KCM settings UI (not the daemon hot
    // path), so briefly blocking the caller thread is acceptable.
    constexpr int WindowListTimeoutMs = 2000;
    QTimer::singleShot(WindowListTimeoutMs, &loop, &QEventLoop::quit);

    // Signal the KWin effect to enumerate windows
    Q_EMIT runningWindowsRequested();

    // Block until provideRunningWindows() is called or timeout
    loop.exec();

    m_windowListLoop = nullptr;
    return m_pendingWindowList;
}

void SettingsAdaptor::provideRunningWindows(const QString& json)
{
    m_pendingWindowList = json;
    if (m_windowListLoop && m_windowListLoop->isRunning()) {
        m_windowListLoop->quit();
    }
}

} // namespace PlasmaZones
