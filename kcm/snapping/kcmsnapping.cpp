// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kcmsnapping.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDir>
#include "../common/dbusutils.h"
#include <QFile>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>
#include <QStandardPaths>
#include <KPluginFactory>
#include "../../src/config/configdefaults.h"
#include "../../src/config/settings.h"
#include "../../src/core/constants.h"
#include "../../src/core/interfaces.h"
#include "../../src/core/modifierutils.h"
#include "../../src/core/utils.h"

K_PLUGIN_CLASS_WITH_JSON(PlasmaZones::KCMSnapping, "kcm_plasmazones_snapping.json")

namespace PlasmaZones {

KCMSnapping::KCMSnapping(QObject* parent, const KPluginMetaData& data)
    : KQuickConfigModule(parent, data)
{
    m_settings = new Settings(this);
    setButtons(Apply | Default);

    refreshScreens();

    // Listen for screen changes from the daemon
    QDBusConnection::sessionBus().connect(QString(DBus::ServiceName), QString(DBus::ObjectPath),
                                          QString(DBus::Interface::Screen), QStringLiteral("screenAdded"), this,
                                          SLOT(refreshScreens()));
    QDBusConnection::sessionBus().connect(QString(DBus::ServiceName), QString(DBus::ObjectPath),
                                          QString(DBus::Interface::Screen), QStringLiteral("screenRemoved"), this,
                                          SLOT(refreshScreens()));
}

// ── Load / Save ─────────────────────────────────────────────────────────

void KCMSnapping::load()
{
    KQuickConfigModule::load();
    m_settings->load();
    refreshScreens();
    emitAllChanged();
    setNeedsSave(false);
}

void KCMSnapping::save()
{
    m_settings->save();

    KCMDBus::notifyReload();

    KQuickConfigModule::save();
    setNeedsSave(false);
}

void KCMSnapping::defaults()
{
    KQuickConfigModule::defaults();

    // Reset all managed properties to ConfigDefaults values
    setSnappingEnabled(ConfigDefaults::snappingEnabled());
    setDragActivationTriggers(convertTriggersForQml(ConfigDefaults::dragActivationTriggers()));
    setToggleActivation(ConfigDefaults::toggleActivation());
    setZoneSpanEnabled(ConfigDefaults::zoneSpanEnabled());
    setZoneSpanTriggers(convertTriggersForQml(ConfigDefaults::zoneSpanTriggers()));
    setSnapAssistFeatureEnabled(ConfigDefaults::snapAssistFeatureEnabled());
    setSnapAssistEnabled(ConfigDefaults::snapAssistEnabled());
    setSnapAssistTriggers(convertTriggersForQml(ConfigDefaults::snapAssistTriggers()));

    setShowZonesOnAllMonitors(false);
    setShowZoneNumbers(true);
    setFlashZonesOnSwitch(true);
    setKeepWindowsInZonesOnResolutionChange(ConfigDefaults::keepWindowsInZonesOnResolutionChange());
    setMoveNewWindowsToLastZone(ConfigDefaults::moveNewWindowsToLastZone());
    setRestoreOriginalSizeOnUnsnap(ConfigDefaults::restoreOriginalSizeOnUnsnap());
    setStickyWindowHandling(ConfigDefaults::stickyWindowHandling());
    setRestoreWindowsToZonesOnLogin(ConfigDefaults::restoreWindowsToZonesOnLogin());

    setUseSystemColors(ConfigDefaults::useSystemColors());
    setHighlightColor(ConfigDefaults::highlightColor());
    setInactiveColor(ConfigDefaults::inactiveColor());
    setBorderColor(ConfigDefaults::borderColor());
    setLabelFontColor(ConfigDefaults::labelFontColor());
    setActiveOpacity(ConfigDefaults::activeOpacity());
    setInactiveOpacity(ConfigDefaults::inactiveOpacity());
    setBorderWidth(ConfigDefaults::borderWidth());
    setBorderRadius(ConfigDefaults::borderRadius());
    setEnableBlur(ConfigDefaults::enableBlur());
    setLabelFontFamily(ConfigDefaults::labelFontFamily());
    setLabelFontSizeScale(ConfigDefaults::labelFontSizeScale());
    setLabelFontWeight(ConfigDefaults::labelFontWeight());
    setLabelFontItalic(ConfigDefaults::labelFontItalic());
    setLabelFontUnderline(ConfigDefaults::labelFontUnderline());
    setLabelFontStrikeout(ConfigDefaults::labelFontStrikeout());

    setEnableShaderEffects(ConfigDefaults::enableShaderEffects());
    setShaderFrameRate(ConfigDefaults::shaderFrameRate());
    setEnableAudioVisualizer(ConfigDefaults::enableAudioVisualizer());
    setAudioSpectrumBarCount(ConfigDefaults::audioSpectrumBarCount());

    setZonePadding(ConfigDefaults::zonePadding());
    setOuterGap(ConfigDefaults::outerGap());
    setUsePerSideOuterGap(ConfigDefaults::usePerSideOuterGap());
    setOuterGapTop(ConfigDefaults::outerGapTop());
    setOuterGapBottom(ConfigDefaults::outerGapBottom());
    setOuterGapLeft(ConfigDefaults::outerGapLeft());
    setOuterGapRight(ConfigDefaults::outerGapRight());
    setAdjacentThreshold(ConfigDefaults::adjacentThreshold());

    setZoneSelectorEnabled(ConfigDefaults::zoneSelectorEnabled());
    setZoneSelectorTriggerDistance(ConfigDefaults::triggerDistance());
    setZoneSelectorPosition(ConfigDefaults::position());
    setZoneSelectorLayoutMode(ConfigDefaults::layoutMode());
    setZoneSelectorPreviewWidth(ConfigDefaults::previewWidth());
    setZoneSelectorPreviewHeight(ConfigDefaults::previewHeight());
    setZoneSelectorPreviewLockAspect(ConfigDefaults::previewLockAspect());
    setZoneSelectorGridColumns(ConfigDefaults::gridColumns());
    setZoneSelectorSizeMode(ConfigDefaults::sizeMode());
    setZoneSelectorMaxRows(ConfigDefaults::maxRows());
}

void KCMSnapping::emitAllChanged()
{
    Q_EMIT dragActivationTriggersChanged();
    Q_EMIT alwaysActivateOnDragChanged();
    Q_EMIT toggleActivationChanged();
    Q_EMIT snappingEnabledChanged();
    Q_EMIT zoneSpanEnabledChanged();
    Q_EMIT zoneSpanTriggersChanged();
    Q_EMIT snapAssistFeatureEnabledChanged();
    Q_EMIT snapAssistEnabledChanged();
    Q_EMIT snapAssistTriggersChanged();
    Q_EMIT showZonesOnAllMonitorsChanged();
    Q_EMIT showZoneNumbersChanged();
    Q_EMIT flashZonesOnSwitchChanged();
    Q_EMIT keepWindowsInZonesOnResolutionChangeChanged();
    Q_EMIT moveNewWindowsToLastZoneChanged();
    Q_EMIT restoreOriginalSizeOnUnsnapChanged();
    Q_EMIT stickyWindowHandlingChanged();
    Q_EMIT restoreWindowsToZonesOnLoginChanged();
    Q_EMIT useSystemColorsChanged();
    Q_EMIT highlightColorChanged();
    Q_EMIT inactiveColorChanged();
    Q_EMIT borderColorChanged();
    Q_EMIT labelFontColorChanged();
    Q_EMIT activeOpacityChanged();
    Q_EMIT inactiveOpacityChanged();
    Q_EMIT borderWidthChanged();
    Q_EMIT borderRadiusChanged();
    Q_EMIT enableBlurChanged();
    Q_EMIT labelFontFamilyChanged();
    Q_EMIT labelFontSizeScaleChanged();
    Q_EMIT labelFontWeightChanged();
    Q_EMIT labelFontItalicChanged();
    Q_EMIT labelFontUnderlineChanged();
    Q_EMIT labelFontStrikeoutChanged();
    Q_EMIT enableShaderEffectsChanged();
    Q_EMIT shaderFrameRateChanged();
    Q_EMIT enableAudioVisualizerChanged();
    Q_EMIT audioSpectrumBarCountChanged();
    Q_EMIT zonePaddingChanged();
    Q_EMIT outerGapChanged();
    Q_EMIT usePerSideOuterGapChanged();
    Q_EMIT outerGapTopChanged();
    Q_EMIT outerGapBottomChanged();
    Q_EMIT outerGapLeftChanged();
    Q_EMIT outerGapRightChanged();
    Q_EMIT adjacentThresholdChanged();
    Q_EMIT zoneSelectorEnabledChanged();
    Q_EMIT zoneSelectorTriggerDistanceChanged();
    Q_EMIT zoneSelectorPositionChanged();
    Q_EMIT zoneSelectorLayoutModeChanged();
    Q_EMIT zoneSelectorPreviewWidthChanged();
    Q_EMIT zoneSelectorPreviewHeightChanged();
    Q_EMIT zoneSelectorPreviewLockAspectChanged();
    Q_EMIT zoneSelectorGridColumnsChanged();
    Q_EMIT zoneSelectorSizeModeChanged();
    Q_EMIT zoneSelectorMaxRowsChanged();
    Q_EMIT screensChanged();
}

// ── Trigger conversion helpers ──────────────────────────────────────────

QVariantList KCMSnapping::convertTriggersForQml(const QVariantList& triggers)
{
    QVariantList result;
    for (const auto& t : triggers) {
        auto map = t.toMap();
        QVariantMap converted;
        converted[QStringLiteral("modifier")] =
            ModifierUtils::dragModifierToBitmask(map.value(QStringLiteral("modifier"), 0).toInt());
        converted[QStringLiteral("mouseButton")] = map.value(QStringLiteral("mouseButton"), 0);
        result.append(converted);
    }
    return result;
}

QVariantList KCMSnapping::convertTriggersForStorage(const QVariantList& triggers)
{
    QVariantList result;
    for (const auto& t : triggers) {
        auto map = t.toMap();
        QVariantMap stored;
        stored[QStringLiteral("modifier")] =
            ModifierUtils::bitmaskToDragModifier(map.value(QStringLiteral("modifier"), 0).toInt());
        stored[QStringLiteral("mouseButton")] = map.value(QStringLiteral("mouseButton"), 0);
        result.append(stored);
    }
    return result;
}

// ── Activation getters ──────────────────────────────────────────────────

QVariantList KCMSnapping::dragActivationTriggers() const
{
    return convertTriggersForQml(m_settings->dragActivationTriggers());
}

QVariantList KCMSnapping::defaultDragActivationTriggers() const
{
    return convertTriggersForQml(ConfigDefaults::dragActivationTriggers());
}

bool KCMSnapping::alwaysActivateOnDrag() const
{
    const int alwaysActive = static_cast<int>(DragModifier::AlwaysActive);
    const auto triggers = m_settings->dragActivationTriggers();
    for (const auto& t : triggers) {
        if (t.toMap().value(QStringLiteral("modifier"), 0).toInt() == alwaysActive) {
            return true;
        }
    }
    return false;
}

bool KCMSnapping::toggleActivation() const
{
    return m_settings->toggleActivation();
}

bool KCMSnapping::snappingEnabled() const
{
    return m_settings->snappingEnabled();
}

bool KCMSnapping::zoneSpanEnabled() const
{
    return m_settings->zoneSpanEnabled();
}

QVariantList KCMSnapping::zoneSpanTriggers() const
{
    return convertTriggersForQml(m_settings->zoneSpanTriggers());
}

QVariantList KCMSnapping::defaultZoneSpanTriggers() const
{
    return convertTriggersForQml(ConfigDefaults::zoneSpanTriggers());
}

// ── Activation setters ──────────────────────────────────────────────────

void KCMSnapping::setDragActivationTriggers(const QVariantList& triggers)
{
    const bool wasAlwaysActive = alwaysActivateOnDrag();
    const QVariantList converted = convertTriggersForStorage(triggers);
    if (m_settings->dragActivationTriggers() != converted) {
        m_settings->setDragActivationTriggers(converted);
        Q_EMIT dragActivationTriggersChanged();
        if (alwaysActivateOnDrag() != wasAlwaysActive) {
            Q_EMIT alwaysActivateOnDragChanged();
        }
        setNeedsSave(true);
    }
}

void KCMSnapping::setAlwaysActivateOnDrag(bool enabled)
{
    if (alwaysActivateOnDrag() == enabled) {
        return;
    }
    if (enabled) {
        // Single AlwaysActive trigger -- written directly in storage format (DragModifier enum)
        QVariantMap trigger;
        trigger[QStringLiteral("modifier")] = static_cast<int>(DragModifier::AlwaysActive);
        trigger[QStringLiteral("mouseButton")] = 0;
        m_settings->setDragActivationTriggers({trigger});
    } else {
        // Revert to default triggers
        m_settings->setDragActivationTriggers(ConfigDefaults::dragActivationTriggers());
    }
    Q_EMIT alwaysActivateOnDragChanged();
    Q_EMIT dragActivationTriggersChanged();
    setNeedsSave(true);
}

void KCMSnapping::setToggleActivation(bool enable)
{
    if (m_settings->toggleActivation() != enable) {
        m_settings->setToggleActivation(enable);
        Q_EMIT toggleActivationChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setSnappingEnabled(bool enabled)
{
    if (m_settings->snappingEnabled() != enabled) {
        m_settings->setSnappingEnabled(enabled);
        Q_EMIT snappingEnabledChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setZoneSpanEnabled(bool enabled)
{
    if (m_settings->zoneSpanEnabled() != enabled) {
        m_settings->setZoneSpanEnabled(enabled);
        Q_EMIT zoneSpanEnabledChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setZoneSpanTriggers(const QVariantList& triggers)
{
    const QVariantList converted = convertTriggersForStorage(triggers);
    if (m_settings->zoneSpanTriggers() != converted) {
        m_settings->setZoneSpanTriggers(converted);
        Q_EMIT zoneSpanTriggersChanged();
        setNeedsSave(true);
    }
}

// ── Snap Assist getters ─────────────────────────────────────────────────

bool KCMSnapping::snapAssistFeatureEnabled() const
{
    return m_settings->snapAssistFeatureEnabled();
}

bool KCMSnapping::snapAssistEnabled() const
{
    return m_settings->snapAssistEnabled();
}

QVariantList KCMSnapping::snapAssistTriggers() const
{
    return convertTriggersForQml(m_settings->snapAssistTriggers());
}

QVariantList KCMSnapping::defaultSnapAssistTriggers() const
{
    return convertTriggersForQml(ConfigDefaults::snapAssistTriggers());
}

// ── Snap Assist setters ─────────────────────────────────────────────────

void KCMSnapping::setSnapAssistFeatureEnabled(bool enabled)
{
    if (m_settings->snapAssistFeatureEnabled() != enabled) {
        m_settings->setSnapAssistFeatureEnabled(enabled);
        Q_EMIT snapAssistFeatureEnabledChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setSnapAssistEnabled(bool enabled)
{
    if (m_settings->snapAssistEnabled() != enabled) {
        m_settings->setSnapAssistEnabled(enabled);
        Q_EMIT snapAssistEnabledChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setSnapAssistTriggers(const QVariantList& triggers)
{
    QVariantList converted = convertTriggersForStorage(triggers);
    if (m_settings->snapAssistTriggers() != converted) {
        m_settings->setSnapAssistTriggers(converted);
        Q_EMIT snapAssistTriggersChanged();
        setNeedsSave(true);
    }
}

// ── Display / Behavior getters ──────────────────────────────────────────

bool KCMSnapping::showZonesOnAllMonitors() const
{
    return m_settings->showZonesOnAllMonitors();
}

bool KCMSnapping::showZoneNumbers() const
{
    return m_settings->showZoneNumbers();
}

bool KCMSnapping::flashZonesOnSwitch() const
{
    return m_settings->flashZonesOnSwitch();
}

bool KCMSnapping::keepWindowsInZonesOnResolutionChange() const
{
    return m_settings->keepWindowsInZonesOnResolutionChange();
}

bool KCMSnapping::moveNewWindowsToLastZone() const
{
    return m_settings->moveNewWindowsToLastZone();
}

bool KCMSnapping::restoreOriginalSizeOnUnsnap() const
{
    return m_settings->restoreOriginalSizeOnUnsnap();
}

int KCMSnapping::stickyWindowHandling() const
{
    return static_cast<int>(m_settings->stickyWindowHandling());
}

bool KCMSnapping::restoreWindowsToZonesOnLogin() const
{
    return m_settings->restoreWindowsToZonesOnLogin();
}

// ── Display / Behavior setters ──────────────────────────────────────────

void KCMSnapping::setShowZonesOnAllMonitors(bool show)
{
    if (m_settings->showZonesOnAllMonitors() != show) {
        m_settings->setShowZonesOnAllMonitors(show);
        Q_EMIT showZonesOnAllMonitorsChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setShowZoneNumbers(bool show)
{
    if (m_settings->showZoneNumbers() != show) {
        m_settings->setShowZoneNumbers(show);
        Q_EMIT showZoneNumbersChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setFlashZonesOnSwitch(bool flash)
{
    if (m_settings->flashZonesOnSwitch() != flash) {
        m_settings->setFlashZonesOnSwitch(flash);
        Q_EMIT flashZonesOnSwitchChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setKeepWindowsInZonesOnResolutionChange(bool keep)
{
    if (m_settings->keepWindowsInZonesOnResolutionChange() != keep) {
        m_settings->setKeepWindowsInZonesOnResolutionChange(keep);
        Q_EMIT keepWindowsInZonesOnResolutionChangeChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setMoveNewWindowsToLastZone(bool move)
{
    if (m_settings->moveNewWindowsToLastZone() != move) {
        m_settings->setMoveNewWindowsToLastZone(move);
        Q_EMIT moveNewWindowsToLastZoneChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setRestoreOriginalSizeOnUnsnap(bool restore)
{
    if (m_settings->restoreOriginalSizeOnUnsnap() != restore) {
        m_settings->setRestoreOriginalSizeOnUnsnap(restore);
        Q_EMIT restoreOriginalSizeOnUnsnapChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setStickyWindowHandling(int handling)
{
    int clamped = qBound(static_cast<int>(StickyWindowHandling::TreatAsNormal), handling,
                         static_cast<int>(StickyWindowHandling::IgnoreAll));
    if (static_cast<int>(m_settings->stickyWindowHandling()) != clamped) {
        m_settings->setStickyWindowHandling(static_cast<StickyWindowHandling>(clamped));
        Q_EMIT stickyWindowHandlingChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setRestoreWindowsToZonesOnLogin(bool restore)
{
    if (m_settings->restoreWindowsToZonesOnLogin() != restore) {
        m_settings->setRestoreWindowsToZonesOnLogin(restore);
        Q_EMIT restoreWindowsToZonesOnLoginChanged();
        setNeedsSave(true);
    }
}

// ── Appearance getters ──────────────────────────────────────────────────

bool KCMSnapping::useSystemColors() const
{
    return m_settings->useSystemColors();
}

QColor KCMSnapping::highlightColor() const
{
    return m_settings->highlightColor();
}

QColor KCMSnapping::inactiveColor() const
{
    return m_settings->inactiveColor();
}

QColor KCMSnapping::borderColor() const
{
    return m_settings->borderColor();
}

QColor KCMSnapping::labelFontColor() const
{
    return m_settings->labelFontColor();
}

qreal KCMSnapping::activeOpacity() const
{
    return m_settings->activeOpacity();
}

qreal KCMSnapping::inactiveOpacity() const
{
    return m_settings->inactiveOpacity();
}

int KCMSnapping::borderWidth() const
{
    return m_settings->borderWidth();
}

int KCMSnapping::borderRadius() const
{
    return m_settings->borderRadius();
}

bool KCMSnapping::enableBlur() const
{
    return m_settings->enableBlur();
}

QString KCMSnapping::labelFontFamily() const
{
    return m_settings->labelFontFamily();
}

qreal KCMSnapping::labelFontSizeScale() const
{
    return m_settings->labelFontSizeScale();
}

int KCMSnapping::labelFontWeight() const
{
    return m_settings->labelFontWeight();
}

bool KCMSnapping::labelFontItalic() const
{
    return m_settings->labelFontItalic();
}

bool KCMSnapping::labelFontUnderline() const
{
    return m_settings->labelFontUnderline();
}

bool KCMSnapping::labelFontStrikeout() const
{
    return m_settings->labelFontStrikeout();
}

// ── Appearance setters ──────────────────────────────────────────────────

void KCMSnapping::setUseSystemColors(bool use)
{
    if (m_settings->useSystemColors() != use) {
        m_settings->setUseSystemColors(use);
        Q_EMIT useSystemColorsChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setHighlightColor(const QColor& color)
{
    if (m_settings->highlightColor() != color) {
        m_settings->setHighlightColor(color);
        Q_EMIT highlightColorChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setInactiveColor(const QColor& color)
{
    if (m_settings->inactiveColor() != color) {
        m_settings->setInactiveColor(color);
        Q_EMIT inactiveColorChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setBorderColor(const QColor& color)
{
    if (m_settings->borderColor() != color) {
        m_settings->setBorderColor(color);
        Q_EMIT borderColorChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setLabelFontColor(const QColor& color)
{
    if (m_settings->labelFontColor() != color) {
        m_settings->setLabelFontColor(color);
        Q_EMIT labelFontColorChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setActiveOpacity(qreal opacity)
{
    if (!qFuzzyCompare(1.0 + m_settings->activeOpacity(), 1.0 + opacity)) {
        m_settings->setActiveOpacity(opacity);
        Q_EMIT activeOpacityChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setInactiveOpacity(qreal opacity)
{
    if (!qFuzzyCompare(1.0 + m_settings->inactiveOpacity(), 1.0 + opacity)) {
        m_settings->setInactiveOpacity(opacity);
        Q_EMIT inactiveOpacityChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setBorderWidth(int width)
{
    if (m_settings->borderWidth() != width) {
        m_settings->setBorderWidth(width);
        Q_EMIT borderWidthChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setBorderRadius(int radius)
{
    if (m_settings->borderRadius() != radius) {
        m_settings->setBorderRadius(radius);
        Q_EMIT borderRadiusChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setEnableBlur(bool enable)
{
    if (m_settings->enableBlur() != enable) {
        m_settings->setEnableBlur(enable);
        Q_EMIT enableBlurChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setLabelFontFamily(const QString& family)
{
    if (m_settings->labelFontFamily() != family) {
        m_settings->setLabelFontFamily(family);
        Q_EMIT labelFontFamilyChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setLabelFontSizeScale(qreal scale)
{
    scale = qBound(0.25, scale, 3.0);
    if (!qFuzzyCompare(m_settings->labelFontSizeScale(), scale)) {
        m_settings->setLabelFontSizeScale(scale);
        Q_EMIT labelFontSizeScaleChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setLabelFontWeight(int weight)
{
    if (m_settings->labelFontWeight() != weight) {
        m_settings->setLabelFontWeight(weight);
        Q_EMIT labelFontWeightChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setLabelFontItalic(bool italic)
{
    if (m_settings->labelFontItalic() != italic) {
        m_settings->setLabelFontItalic(italic);
        Q_EMIT labelFontItalicChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setLabelFontUnderline(bool underline)
{
    if (m_settings->labelFontUnderline() != underline) {
        m_settings->setLabelFontUnderline(underline);
        Q_EMIT labelFontUnderlineChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setLabelFontStrikeout(bool strikeout)
{
    if (m_settings->labelFontStrikeout() != strikeout) {
        m_settings->setLabelFontStrikeout(strikeout);
        Q_EMIT labelFontStrikeoutChanged();
        setNeedsSave(true);
    }
}

// ── Shader Effects ──────────────────────────────────────────────────────

bool KCMSnapping::enableShaderEffects() const
{
    return m_settings->enableShaderEffects();
}

int KCMSnapping::shaderFrameRate() const
{
    return m_settings->shaderFrameRate();
}

bool KCMSnapping::enableAudioVisualizer() const
{
    return m_settings->enableAudioVisualizer();
}

bool KCMSnapping::cavaAvailable() const
{
    return !QStandardPaths::findExecutable(QStringLiteral("cava")).isEmpty();
}

int KCMSnapping::audioSpectrumBarCount() const
{
    return m_settings->audioSpectrumBarCount();
}

void KCMSnapping::setEnableShaderEffects(bool enable)
{
    if (m_settings->enableShaderEffects() != enable) {
        m_settings->setEnableShaderEffects(enable);
        Q_EMIT enableShaderEffectsChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setShaderFrameRate(int fps)
{
    if (m_settings->shaderFrameRate() != fps) {
        m_settings->setShaderFrameRate(fps);
        Q_EMIT shaderFrameRateChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setEnableAudioVisualizer(bool enable)
{
    if (m_settings->enableAudioVisualizer() != enable) {
        m_settings->setEnableAudioVisualizer(enable);
        Q_EMIT enableAudioVisualizerChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setAudioSpectrumBarCount(int count)
{
    // CAVA requires even bar count for stereo output
    const int even = (count % 2 != 0) ? count + 1 : count;
    const int clamped = qBound(Audio::MinBars, even, Audio::MaxBars);
    if (m_settings->audioSpectrumBarCount() != clamped) {
        m_settings->setAudioSpectrumBarCount(clamped);
        Q_EMIT audioSpectrumBarCountChanged();
        setNeedsSave(true);
    }
}

// ── Zones / Gaps getters ────────────────────────────────────────────────

int KCMSnapping::zonePadding() const
{
    return m_settings->zonePadding();
}

int KCMSnapping::outerGap() const
{
    return m_settings->outerGap();
}

bool KCMSnapping::usePerSideOuterGap() const
{
    return m_settings->usePerSideOuterGap();
}

int KCMSnapping::outerGapTop() const
{
    return m_settings->outerGapTop();
}

int KCMSnapping::outerGapBottom() const
{
    return m_settings->outerGapBottom();
}

int KCMSnapping::outerGapLeft() const
{
    return m_settings->outerGapLeft();
}

int KCMSnapping::outerGapRight() const
{
    return m_settings->outerGapRight();
}

int KCMSnapping::adjacentThreshold() const
{
    return m_settings->adjacentThreshold();
}

// ── Zones / Gaps setters ────────────────────────────────────────────────

void KCMSnapping::setZonePadding(int padding)
{
    if (m_settings->zonePadding() != padding) {
        m_settings->setZonePadding(padding);
        Q_EMIT zonePaddingChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setOuterGap(int gap)
{
    if (m_settings->outerGap() != gap) {
        m_settings->setOuterGap(gap);
        Q_EMIT outerGapChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setUsePerSideOuterGap(bool enabled)
{
    if (m_settings->usePerSideOuterGap() != enabled) {
        m_settings->setUsePerSideOuterGap(enabled);
        Q_EMIT usePerSideOuterGapChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setOuterGapTop(int gap)
{
    if (m_settings->outerGapTop() != gap) {
        m_settings->setOuterGapTop(gap);
        Q_EMIT outerGapTopChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setOuterGapBottom(int gap)
{
    if (m_settings->outerGapBottom() != gap) {
        m_settings->setOuterGapBottom(gap);
        Q_EMIT outerGapBottomChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setOuterGapLeft(int gap)
{
    if (m_settings->outerGapLeft() != gap) {
        m_settings->setOuterGapLeft(gap);
        Q_EMIT outerGapLeftChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setOuterGapRight(int gap)
{
    if (m_settings->outerGapRight() != gap) {
        m_settings->setOuterGapRight(gap);
        Q_EMIT outerGapRightChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setAdjacentThreshold(int threshold)
{
    if (m_settings->adjacentThreshold() != threshold) {
        m_settings->setAdjacentThreshold(threshold);
        Q_EMIT adjacentThresholdChanged();
        setNeedsSave(true);
    }
}

// ── Zone Selector getters ───────────────────────────────────────────────

bool KCMSnapping::zoneSelectorEnabled() const
{
    return m_settings->zoneSelectorEnabled();
}

int KCMSnapping::zoneSelectorTriggerDistance() const
{
    return m_settings->zoneSelectorTriggerDistance();
}

int KCMSnapping::zoneSelectorPosition() const
{
    return m_settings->zoneSelectorPositionInt();
}

int KCMSnapping::zoneSelectorLayoutMode() const
{
    return m_settings->zoneSelectorLayoutModeInt();
}

int KCMSnapping::zoneSelectorPreviewWidth() const
{
    return m_settings->zoneSelectorPreviewWidth();
}

int KCMSnapping::zoneSelectorPreviewHeight() const
{
    return m_settings->zoneSelectorPreviewHeight();
}

bool KCMSnapping::zoneSelectorPreviewLockAspect() const
{
    return m_settings->zoneSelectorPreviewLockAspect();
}

int KCMSnapping::zoneSelectorGridColumns() const
{
    return m_settings->zoneSelectorGridColumns();
}

int KCMSnapping::zoneSelectorSizeMode() const
{
    return m_settings->zoneSelectorSizeModeInt();
}

int KCMSnapping::zoneSelectorMaxRows() const
{
    return m_settings->zoneSelectorMaxRows();
}

// ── Zone Selector setters ───────────────────────────────────────────────

void KCMSnapping::setZoneSelectorEnabled(bool enabled)
{
    if (m_settings->zoneSelectorEnabled() != enabled) {
        m_settings->setZoneSelectorEnabled(enabled);
        Q_EMIT zoneSelectorEnabledChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setZoneSelectorTriggerDistance(int distance)
{
    if (m_settings->zoneSelectorTriggerDistance() != distance) {
        m_settings->setZoneSelectorTriggerDistance(distance);
        Q_EMIT zoneSelectorTriggerDistanceChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setZoneSelectorPosition(int position)
{
    if (m_settings->zoneSelectorPositionInt() != position) {
        m_settings->setZoneSelectorPositionInt(position);
        Q_EMIT zoneSelectorPositionChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setZoneSelectorLayoutMode(int mode)
{
    if (m_settings->zoneSelectorLayoutModeInt() != mode) {
        m_settings->setZoneSelectorLayoutModeInt(mode);
        Q_EMIT zoneSelectorLayoutModeChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setZoneSelectorPreviewWidth(int width)
{
    if (m_settings->zoneSelectorPreviewWidth() != width) {
        m_settings->setZoneSelectorPreviewWidth(width);
        Q_EMIT zoneSelectorPreviewWidthChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setZoneSelectorPreviewHeight(int height)
{
    if (m_settings->zoneSelectorPreviewHeight() != height) {
        m_settings->setZoneSelectorPreviewHeight(height);
        Q_EMIT zoneSelectorPreviewHeightChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setZoneSelectorPreviewLockAspect(bool locked)
{
    if (m_settings->zoneSelectorPreviewLockAspect() != locked) {
        m_settings->setZoneSelectorPreviewLockAspect(locked);
        Q_EMIT zoneSelectorPreviewLockAspectChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setZoneSelectorGridColumns(int columns)
{
    if (m_settings->zoneSelectorGridColumns() != columns) {
        m_settings->setZoneSelectorGridColumns(columns);
        Q_EMIT zoneSelectorGridColumnsChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setZoneSelectorSizeMode(int mode)
{
    if (m_settings->zoneSelectorSizeModeInt() != mode) {
        m_settings->setZoneSelectorSizeModeInt(mode);
        Q_EMIT zoneSelectorSizeModeChanged();
        setNeedsSave(true);
    }
}

void KCMSnapping::setZoneSelectorMaxRows(int rows)
{
    if (m_settings->zoneSelectorMaxRows() != rows) {
        m_settings->setZoneSelectorMaxRows(rows);
        Q_EMIT zoneSelectorMaxRowsChanged();
        setNeedsSave(true);
    }
}

// ── Screens ─────────────────────────────────────────────────────────────

QVariantList KCMSnapping::screens() const
{
    return m_screens;
}

void KCMSnapping::refreshScreens()
{
    QVariantList newScreens;

    // Get primary screen name from daemon for isPrimary flag
    QString primaryScreenName;
    QDBusMessage primaryReply =
        KCMDBus::callDaemon(QString(DBus::Interface::Screen), QStringLiteral("getPrimaryScreen"));
    if (primaryReply.type() == QDBusMessage::ReplyMessage && !primaryReply.arguments().isEmpty()) {
        primaryScreenName = primaryReply.arguments().first().toString();
    }

    // Get screens from daemon via D-Bus
    QDBusMessage screenReply = KCMDBus::callDaemon(QString(DBus::Interface::Screen), QStringLiteral("getScreens"));

    if (screenReply.type() == QDBusMessage::ReplyMessage && !screenReply.arguments().isEmpty()) {
        QStringList screenNames = screenReply.arguments().first().toStringList();

        for (const QString& screenName : screenNames) {
            // Get screen info
            QDBusMessage infoReply =
                KCMDBus::callDaemon(QString(DBus::Interface::Screen), QStringLiteral("getScreenInfo"), {screenName});

            if (infoReply.type() == QDBusMessage::ReplyMessage && !infoReply.arguments().isEmpty()) {
                QString infoJson = infoReply.arguments().first().toString();
                QJsonDocument doc = QJsonDocument::fromJson(infoJson.toUtf8());
                if (!doc.isNull() && doc.isObject()) {
                    QJsonObject jsonObj = doc.object();
                    QVariantMap screenInfo;
                    screenInfo[QStringLiteral("name")] = screenName;
                    screenInfo[QStringLiteral("isPrimary")] = (screenName == primaryScreenName);

                    // Include stable EDID-based screen ID from daemon
                    if (jsonObj.contains(JsonKeys::ScreenId)) {
                        screenInfo[QStringLiteral("screenId")] = jsonObj[JsonKeys::ScreenId].toString();
                    } else {
                        screenInfo[QStringLiteral("screenId")] = screenName;
                    }

                    // Forward manufacturer/model for display
                    if (jsonObj.contains(JsonKeys::Manufacturer)) {
                        screenInfo[QStringLiteral("manufacturer")] = jsonObj[JsonKeys::Manufacturer].toString();
                    }
                    if (jsonObj.contains(JsonKeys::Model)) {
                        screenInfo[QStringLiteral("model")] = jsonObj[JsonKeys::Model].toString();
                    }

                    // Create resolution string from geometry for QML display
                    if (jsonObj.contains(JsonKeys::Geometry)) {
                        QJsonObject geom = jsonObj[JsonKeys::Geometry].toObject();
                        int width = geom[JsonKeys::Width].toInt();
                        int height = geom[JsonKeys::Height].toInt();
                        screenInfo[QStringLiteral("resolution")] = QStringLiteral("%1\u00d7%2").arg(width).arg(height);
                    }

                    newScreens.append(screenInfo);
                } else {
                    QVariantMap screenInfo;
                    screenInfo[QStringLiteral("name")] = screenName;
                    screenInfo[QStringLiteral("isPrimary")] = (screenName == primaryScreenName);
                    newScreens.append(screenInfo);
                }
            } else {
                QVariantMap screenInfo;
                screenInfo[QStringLiteral("name")] = screenName;
                screenInfo[QStringLiteral("isPrimary")] = (screenName == primaryScreenName);
                newScreens.append(screenInfo);
            }
        }
    }

    // Fallback: if no screens from daemon, get from Qt
    if (newScreens.isEmpty()) {
        QScreen* primaryScreen = QGuiApplication::primaryScreen();
        for (QScreen* screen : QGuiApplication::screens()) {
            QVariantMap screenInfo;
            screenInfo[QStringLiteral("name")] = screen->name();
            screenInfo[QStringLiteral("isPrimary")] = (screen == primaryScreen);
            screenInfo[QStringLiteral("resolution")] =
                QStringLiteral("%1\u00d7%2").arg(screen->geometry().width()).arg(screen->geometry().height());
            screenInfo[QStringLiteral("manufacturer")] = screen->manufacturer();
            screenInfo[QStringLiteral("model")] = screen->model();
            screenInfo[QStringLiteral("screenId")] = screen->name();
            newScreens.append(screenInfo);
        }
    }

    m_screens = newScreens;
    Q_EMIT screensChanged();
}

// ── Font helpers ────────────────────────────────────────────────────────

QStringList KCMSnapping::fontStylesForFamily(const QString& family) const
{
    return QFontDatabase::styles(family);
}

int KCMSnapping::fontStyleWeight(const QString& family, const QString& style) const
{
    return QFontDatabase::weight(family, style);
}

bool KCMSnapping::fontStyleItalic(const QString& family, const QString& style) const
{
    return QFontDatabase::italic(family, style);
}

// ── Color import ────────────────────────────────────────────────────────

void KCMSnapping::loadColorsFromPywal()
{
    QString pywalPath = QDir::homePath() + QStringLiteral("/.cache/wal/colors.json");
    if (!QFile::exists(pywalPath)) {
        Q_EMIT colorImportError(
            tr("Pywal colors not found. Run 'wal' to generate colors first.\n\nExpected file: %1").arg(pywalPath));
        return;
    }

    QString error = m_settings->loadColorsFromFile(pywalPath);
    if (!error.isEmpty()) {
        Q_EMIT colorImportError(error);
        return;
    }

    Q_EMIT highlightColorChanged();
    Q_EMIT inactiveColorChanged();
    Q_EMIT borderColorChanged();
    Q_EMIT labelFontColorChanged();
    Q_EMIT useSystemColorsChanged();
    setNeedsSave(true);
}

void KCMSnapping::loadColorsFromFile(const QString& filePath)
{
    QString error = m_settings->loadColorsFromFile(filePath);
    if (!error.isEmpty()) {
        Q_EMIT colorImportError(error);
        return;
    }

    Q_EMIT highlightColorChanged();
    Q_EMIT inactiveColorChanged();
    Q_EMIT borderColorChanged();
    Q_EMIT labelFontColorChanged();
    Q_EMIT useSystemColorsChanged();
    setNeedsSave(true);
}

// ── Per-screen settings helpers ─────────────────────────────────────────

QVariantMap KCMSnapping::getPerScreenSettingsImpl(const QString& screenName, PerScreenGetter getter) const
{
    return m_settings ? (m_settings->*getter)(Utils::screenIdForName(screenName)) : QVariantMap();
}

void KCMSnapping::setPerScreenSettingImpl(const QString& screenName, const QString& key, const QVariant& value,
                                          PerScreenSetter setter)
{
    if (!m_settings || screenName.isEmpty()) {
        return;
    }
    (m_settings->*setter)(Utils::screenIdForName(screenName), key, value);
    setNeedsSave(true);
}

void KCMSnapping::clearPerScreenSettingsImpl(const QString& screenName, PerScreenClearer clearer)
{
    if (!m_settings || screenName.isEmpty()) {
        return;
    }
    (m_settings->*clearer)(Utils::screenIdForName(screenName));
    setNeedsSave(true);
}

bool KCMSnapping::hasPerScreenSettingsImpl(const QString& screenName, PerScreenChecker checker) const
{
    return m_settings ? (m_settings->*checker)(Utils::screenIdForName(screenName)) : false;
}

// Per-screen snapping
QVariantMap KCMSnapping::getPerScreenSnappingSettings(const QString& screenName) const
{
    return getPerScreenSettingsImpl(screenName, &Settings::getPerScreenSnappingSettings);
}
void KCMSnapping::setPerScreenSnappingSetting(const QString& screenName, const QString& key, const QVariant& value)
{
    setPerScreenSettingImpl(screenName, key, value, &Settings::setPerScreenSnappingSetting);
}
void KCMSnapping::clearPerScreenSnappingSettings(const QString& screenName)
{
    clearPerScreenSettingsImpl(screenName, &Settings::clearPerScreenSnappingSettings);
}
bool KCMSnapping::hasPerScreenSnappingSettings(const QString& screenName) const
{
    return hasPerScreenSettingsImpl(screenName, &Settings::hasPerScreenSnappingSettings);
}

// Per-screen zone selector
QVariantMap KCMSnapping::getPerScreenZoneSelectorSettings(const QString& screenName) const
{
    return getPerScreenSettingsImpl(screenName, &Settings::getPerScreenZoneSelectorSettings);
}
void KCMSnapping::setPerScreenZoneSelectorSetting(const QString& screenName, const QString& key, const QVariant& value)
{
    setPerScreenSettingImpl(screenName, key, value, &Settings::setPerScreenZoneSelectorSetting);
}
void KCMSnapping::clearPerScreenZoneSelectorSettings(const QString& screenName)
{
    clearPerScreenSettingsImpl(screenName, &Settings::clearPerScreenZoneSelectorSettings);
}
bool KCMSnapping::hasPerScreenZoneSelectorSettings(const QString& screenName) const
{
    return hasPerScreenSettingsImpl(screenName, &Settings::hasPerScreenZoneSelectorSettings);
}

// ── Monitor disable ─────────────────────────────────────────────────────

bool KCMSnapping::isMonitorDisabled(const QString& screenName) const
{
    return m_settings && m_settings->isMonitorDisabled(screenName);
}

void KCMSnapping::setMonitorDisabled(const QString& screenName, bool disabled)
{
    if (!m_settings || screenName.isEmpty()) {
        return;
    }
    // Translate connector name to stable EDID-based screen ID for storage
    QString id = Utils::screenIdForName(screenName);
    QStringList list = m_settings->disabledMonitors();
    if (disabled) {
        if (!list.contains(id)) {
            list.append(id);
            m_settings->setDisabledMonitors(list);
            setNeedsSave(true);
        }
    } else {
        // Remove both screen ID and any legacy connector name entries
        bool changed = list.removeAll(id) > 0;
        if (id != screenName) {
            changed |= list.removeAll(screenName) > 0;
        }
        if (changed) {
            m_settings->setDisabledMonitors(list);
            setNeedsSave(true);
        }
    }
}

} // namespace PlasmaZones

#include "kcmsnapping.moc"
