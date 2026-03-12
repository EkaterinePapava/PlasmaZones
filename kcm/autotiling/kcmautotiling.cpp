// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kcmautotiling.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QGuiApplication>
#include "../common/dbusutils.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>
#include <KPluginFactory>
#include "../../src/config/configdefaults.h"
#include "../../src/config/settings.h"
#include "../../src/core/constants.h"
#include "../../src/core/interfaces.h"
#include "../../src/core/utils.h"
#include "../../src/autotile/AlgorithmRegistry.h"
#include "../../src/autotile/TilingAlgorithm.h"
#include "../../src/autotile/TilingState.h"

K_PLUGIN_CLASS_WITH_JSON(PlasmaZones::KCMAutotiling, "kcm_plasmazones_autotiling.json")

namespace PlasmaZones {

KCMAutotiling::KCMAutotiling(QObject* parent, const KPluginMetaData& data)
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

void KCMAutotiling::load()
{
    KQuickConfigModule::load();
    m_settings->load();
    refreshScreens();
    emitAllChanged();
    setNeedsSave(false);
}

void KCMAutotiling::save()
{
    m_settings->save();

    KCMDBus::notifyReload();

    KQuickConfigModule::save();
    setNeedsSave(false);
}

void KCMAutotiling::defaults()
{
    KQuickConfigModule::defaults();

    setAutotileEnabled(ConfigDefaults::autotileEnabled());
    setAutotileAlgorithm(ConfigDefaults::autotileAlgorithm());
    setAutotileSplitRatio(ConfigDefaults::autotileSplitRatio());
    setAutotileMasterCount(ConfigDefaults::autotileMasterCount());
    setAutotileCenteredMasterSplitRatio(ConfigDefaults::autotileCenteredMasterSplitRatio());
    setAutotileCenteredMasterMasterCount(ConfigDefaults::autotileCenteredMasterMasterCount());
    setAutotileInnerGap(ConfigDefaults::autotileInnerGap());
    setAutotileOuterGap(ConfigDefaults::autotileOuterGap());
    setAutotileUsePerSideOuterGap(ConfigDefaults::autotileUsePerSideOuterGap());
    setAutotileOuterGapTop(ConfigDefaults::autotileOuterGapTop());
    setAutotileOuterGapBottom(ConfigDefaults::autotileOuterGapBottom());
    setAutotileOuterGapLeft(ConfigDefaults::autotileOuterGapLeft());
    setAutotileOuterGapRight(ConfigDefaults::autotileOuterGapRight());
    setAutotileFocusNewWindows(ConfigDefaults::autotileFocusNewWindows());
    setAutotileSmartGaps(ConfigDefaults::autotileSmartGaps());
    setAutotileMaxWindows(ConfigDefaults::autotileMaxWindows());
    setAutotileInsertPosition(ConfigDefaults::autotileInsertPosition());
    setAutotileFocusFollowsMouse(ConfigDefaults::autotileFocusFollowsMouse());
    setAutotileRespectMinimumSize(ConfigDefaults::autotileRespectMinimumSize());
    setAutotileHideTitleBars(ConfigDefaults::autotileHideTitleBars());
    setAutotileBorderWidth(ConfigDefaults::autotileBorderWidth());
    setAutotileBorderColor(ConfigDefaults::autotileBorderColor());
    setAutotileUseSystemBorderColors(ConfigDefaults::autotileUseSystemBorderColors());
}

void KCMAutotiling::emitAllChanged()
{
    Q_EMIT autotileEnabledChanged();
    Q_EMIT autotileAlgorithmChanged();
    Q_EMIT autotileSplitRatioChanged();
    Q_EMIT autotileMasterCountChanged();
    Q_EMIT autotileCenteredMasterSplitRatioChanged();
    Q_EMIT autotileCenteredMasterMasterCountChanged();
    Q_EMIT autotileInnerGapChanged();
    Q_EMIT autotileOuterGapChanged();
    Q_EMIT autotileSmartGapsChanged();
    Q_EMIT autotileUsePerSideOuterGapChanged();
    Q_EMIT autotileOuterGapTopChanged();
    Q_EMIT autotileOuterGapBottomChanged();
    Q_EMIT autotileOuterGapLeftChanged();
    Q_EMIT autotileOuterGapRightChanged();
    Q_EMIT autotileFocusNewWindowsChanged();
    Q_EMIT autotileMaxWindowsChanged();
    Q_EMIT autotileInsertPositionChanged();
    Q_EMIT autotileFocusFollowsMouseChanged();
    Q_EMIT autotileRespectMinimumSizeChanged();
    Q_EMIT autotileHideTitleBarsChanged();
    Q_EMIT autotileBorderWidthChanged();
    Q_EMIT autotileBorderColorChanged();
    Q_EMIT autotileUseSystemBorderColorsChanged();
    Q_EMIT screensChanged();
}

// ── Enable ──────────────────────────────────────────────────────────────

bool KCMAutotiling::autotileEnabled() const
{
    return m_settings->autotileEnabled();
}

void KCMAutotiling::setAutotileEnabled(bool enabled)
{
    if (m_settings->autotileEnabled() != enabled) {
        m_settings->setAutotileEnabled(enabled);
        Q_EMIT autotileEnabledChanged();
        setNeedsSave(true);
    }
}

// ── Algorithm getters ───────────────────────────────────────────────────

QString KCMAutotiling::autotileAlgorithm() const
{
    return m_settings->autotileAlgorithm();
}

qreal KCMAutotiling::autotileSplitRatio() const
{
    return m_settings->autotileSplitRatio();
}

int KCMAutotiling::autotileMasterCount() const
{
    return m_settings->autotileMasterCount();
}

qreal KCMAutotiling::autotileCenteredMasterSplitRatio() const
{
    return m_settings->autotileCenteredMasterSplitRatio();
}

int KCMAutotiling::autotileCenteredMasterMasterCount() const
{
    return m_settings->autotileCenteredMasterMasterCount();
}

// ── Algorithm setters ───────────────────────────────────────────────────

void KCMAutotiling::setAutotileAlgorithm(const QString& algorithm)
{
    if (m_settings->autotileAlgorithm() != algorithm) {
        m_settings->setAutotileAlgorithm(algorithm);
        Q_EMIT autotileAlgorithmChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileSplitRatio(qreal ratio)
{
    ratio = qBound(0.1, ratio, 0.9);
    if (!qFuzzyCompare(1.0 + m_settings->autotileSplitRatio(), 1.0 + ratio)) {
        m_settings->setAutotileSplitRatio(ratio);
        Q_EMIT autotileSplitRatioChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileMasterCount(int count)
{
    count = qBound(1, count, 5);
    if (m_settings->autotileMasterCount() != count) {
        m_settings->setAutotileMasterCount(count);
        Q_EMIT autotileMasterCountChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileCenteredMasterSplitRatio(qreal ratio)
{
    ratio = qBound(0.1, ratio, 0.9);
    if (!qFuzzyCompare(1.0 + m_settings->autotileCenteredMasterSplitRatio(), 1.0 + ratio)) {
        m_settings->setAutotileCenteredMasterSplitRatio(ratio);
        Q_EMIT autotileCenteredMasterSplitRatioChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileCenteredMasterMasterCount(int count)
{
    count = qBound(1, count, 5);
    if (m_settings->autotileCenteredMasterMasterCount() != count) {
        m_settings->setAutotileCenteredMasterMasterCount(count);
        Q_EMIT autotileCenteredMasterMasterCountChanged();
        setNeedsSave(true);
    }
}

// ── Gaps getters ────────────────────────────────────────────────────────

int KCMAutotiling::autotileInnerGap() const
{
    return m_settings->autotileInnerGap();
}

int KCMAutotiling::autotileOuterGap() const
{
    return m_settings->autotileOuterGap();
}

bool KCMAutotiling::autotileSmartGaps() const
{
    return m_settings->autotileSmartGaps();
}

bool KCMAutotiling::autotileUsePerSideOuterGap() const
{
    return m_settings->autotileUsePerSideOuterGap();
}

int KCMAutotiling::autotileOuterGapTop() const
{
    return m_settings->autotileOuterGapTop();
}

int KCMAutotiling::autotileOuterGapBottom() const
{
    return m_settings->autotileOuterGapBottom();
}

int KCMAutotiling::autotileOuterGapLeft() const
{
    return m_settings->autotileOuterGapLeft();
}

int KCMAutotiling::autotileOuterGapRight() const
{
    return m_settings->autotileOuterGapRight();
}

// ── Gaps setters ────────────────────────────────────────────────────────

void KCMAutotiling::setAutotileInnerGap(int gap)
{
    gap = qBound(0, gap, 50);
    if (m_settings->autotileInnerGap() != gap) {
        m_settings->setAutotileInnerGap(gap);
        Q_EMIT autotileInnerGapChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileOuterGap(int gap)
{
    gap = qBound(0, gap, 50);
    if (m_settings->autotileOuterGap() != gap) {
        m_settings->setAutotileOuterGap(gap);
        Q_EMIT autotileOuterGapChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileSmartGaps(bool smart)
{
    if (m_settings->autotileSmartGaps() != smart) {
        m_settings->setAutotileSmartGaps(smart);
        Q_EMIT autotileSmartGapsChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileUsePerSideOuterGap(bool enabled)
{
    if (m_settings->autotileUsePerSideOuterGap() != enabled) {
        m_settings->setAutotileUsePerSideOuterGap(enabled);
        Q_EMIT autotileUsePerSideOuterGapChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileOuterGapTop(int gap)
{
    gap = qBound(0, gap, 50);
    if (m_settings->autotileOuterGapTop() != gap) {
        m_settings->setAutotileOuterGapTop(gap);
        Q_EMIT autotileOuterGapTopChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileOuterGapBottom(int gap)
{
    gap = qBound(0, gap, 50);
    if (m_settings->autotileOuterGapBottom() != gap) {
        m_settings->setAutotileOuterGapBottom(gap);
        Q_EMIT autotileOuterGapBottomChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileOuterGapLeft(int gap)
{
    gap = qBound(0, gap, 50);
    if (m_settings->autotileOuterGapLeft() != gap) {
        m_settings->setAutotileOuterGapLeft(gap);
        Q_EMIT autotileOuterGapLeftChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileOuterGapRight(int gap)
{
    gap = qBound(0, gap, 50);
    if (m_settings->autotileOuterGapRight() != gap) {
        m_settings->setAutotileOuterGapRight(gap);
        Q_EMIT autotileOuterGapRightChanged();
        setNeedsSave(true);
    }
}

// ── Behavior getters ────────────────────────────────────────────────────

bool KCMAutotiling::autotileFocusNewWindows() const
{
    return m_settings->autotileFocusNewWindows();
}

int KCMAutotiling::autotileMaxWindows() const
{
    return m_settings->autotileMaxWindows();
}

int KCMAutotiling::autotileInsertPosition() const
{
    return m_settings->autotileInsertPositionInt();
}

bool KCMAutotiling::autotileFocusFollowsMouse() const
{
    return m_settings->autotileFocusFollowsMouse();
}

bool KCMAutotiling::autotileRespectMinimumSize() const
{
    return m_settings->autotileRespectMinimumSize();
}

// ── Behavior setters ────────────────────────────────────────────────────

void KCMAutotiling::setAutotileFocusNewWindows(bool focus)
{
    if (m_settings->autotileFocusNewWindows() != focus) {
        m_settings->setAutotileFocusNewWindows(focus);
        Q_EMIT autotileFocusNewWindowsChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileMaxWindows(int max)
{
    max = qBound(1, max, 12);
    if (m_settings->autotileMaxWindows() != max) {
        m_settings->setAutotileMaxWindows(max);
        Q_EMIT autotileMaxWindowsChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileInsertPosition(int position)
{
    position = qBound(0, position, 2);
    if (m_settings->autotileInsertPositionInt() != position) {
        m_settings->setAutotileInsertPositionInt(position);
        Q_EMIT autotileInsertPositionChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileFocusFollowsMouse(bool follows)
{
    if (m_settings->autotileFocusFollowsMouse() != follows) {
        m_settings->setAutotileFocusFollowsMouse(follows);
        Q_EMIT autotileFocusFollowsMouseChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileRespectMinimumSize(bool respect)
{
    if (m_settings->autotileRespectMinimumSize() != respect) {
        m_settings->setAutotileRespectMinimumSize(respect);
        Q_EMIT autotileRespectMinimumSizeChanged();
        setNeedsSave(true);
    }
}

// ── Decorations / Borders getters ───────────────────────────────────────

bool KCMAutotiling::autotileHideTitleBars() const
{
    return m_settings->autotileHideTitleBars();
}

int KCMAutotiling::autotileBorderWidth() const
{
    return m_settings->autotileBorderWidth();
}

QColor KCMAutotiling::autotileBorderColor() const
{
    return m_settings->autotileBorderColor();
}

bool KCMAutotiling::autotileUseSystemBorderColors() const
{
    return m_settings->autotileUseSystemBorderColors();
}

// ── Decorations / Borders setters ───────────────────────────────────────

void KCMAutotiling::setAutotileHideTitleBars(bool hide)
{
    if (m_settings->autotileHideTitleBars() != hide) {
        m_settings->setAutotileHideTitleBars(hide);
        Q_EMIT autotileHideTitleBarsChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileBorderWidth(int width)
{
    width = qBound(0, width, 10);
    if (m_settings->autotileBorderWidth() != width) {
        m_settings->setAutotileBorderWidth(width);
        Q_EMIT autotileBorderWidthChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileBorderColor(const QColor& color)
{
    if (m_settings->autotileBorderColor() != color) {
        m_settings->setAutotileBorderColor(color);
        Q_EMIT autotileBorderColorChanged();
        setNeedsSave(true);
    }
}

void KCMAutotiling::setAutotileUseSystemBorderColors(bool use)
{
    if (m_settings->autotileUseSystemBorderColors() != use) {
        m_settings->setAutotileUseSystemBorderColors(use);
        Q_EMIT autotileUseSystemBorderColorsChanged();
        setNeedsSave(true);
    }
}

// ── Screens ─────────────────────────────────────────────────────────────

QVariantList KCMAutotiling::screens() const
{
    return m_screens;
}

void KCMAutotiling::refreshScreens()
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

// ── Algorithm helpers ───────────────────────────────────────────────────

QVariantList KCMAutotiling::availableAlgorithms() const
{
    QVariantList algorithms;
    auto* registry = AlgorithmRegistry::instance();
    for (const QString& id : registry->availableAlgorithms()) {
        TilingAlgorithm* algo = registry->algorithm(id);
        if (algo) {
            QVariantMap algoMap;
            algoMap[QStringLiteral("id")] = id;
            algoMap[QStringLiteral("name")] = algo->name();
            algoMap[QStringLiteral("description")] = algo->description();
            algoMap[QStringLiteral("defaultMaxWindows")] = algo->defaultMaxWindows();
            algorithms.append(algoMap);
        }
    }
    return algorithms;
}

QVariantList KCMAutotiling::generateAlgorithmPreview(const QString& algorithmId, int windowCount, double splitRatio,
                                                     int masterCount) const
{
    auto* registry = AlgorithmRegistry::instance();
    TilingAlgorithm* algo = registry->algorithm(algorithmId);
    if (!algo) {
        return {};
    }

    const int previewSize = 1000;
    const QRect previewRect(0, 0, previewSize, previewSize);

    TilingState state(QStringLiteral("preview"));
    state.setMasterCount(masterCount);
    state.setSplitRatio(splitRatio);

    const int count = qMax(1, windowCount);
    QVector<QRect> zones = algo->calculateZones({count, previewRect, &state, 0, {}});

    return AlgorithmRegistry::zonesToRelativeGeometry(zones, previewRect);
}

// ── Per-screen settings helpers ─────────────────────────────────────────

QVariantMap KCMAutotiling::getPerScreenSettingsImpl(const QString& screenName, PerScreenGetter getter) const
{
    return m_settings ? (m_settings->*getter)(Utils::screenIdForName(screenName)) : QVariantMap();
}

void KCMAutotiling::setPerScreenSettingImpl(const QString& screenName, const QString& key, const QVariant& value,
                                            PerScreenSetter setter)
{
    if (!m_settings || screenName.isEmpty()) {
        return;
    }
    (m_settings->*setter)(Utils::screenIdForName(screenName), key, value);
    setNeedsSave(true);
}

void KCMAutotiling::clearPerScreenSettingsImpl(const QString& screenName, PerScreenClearer clearer)
{
    if (!m_settings || screenName.isEmpty()) {
        return;
    }
    (m_settings->*clearer)(Utils::screenIdForName(screenName));
    setNeedsSave(true);
}

bool KCMAutotiling::hasPerScreenSettingsImpl(const QString& screenName, PerScreenChecker checker) const
{
    return m_settings ? (m_settings->*checker)(Utils::screenIdForName(screenName)) : false;
}

// Per-screen autotiling
QVariantMap KCMAutotiling::getPerScreenAutotileSettings(const QString& screenName) const
{
    return getPerScreenSettingsImpl(screenName, &Settings::getPerScreenAutotileSettings);
}

void KCMAutotiling::setPerScreenAutotileSetting(const QString& screenName, const QString& key, const QVariant& value)
{
    setPerScreenSettingImpl(screenName, key, value, &Settings::setPerScreenAutotileSetting);
}

void KCMAutotiling::clearPerScreenAutotileSettings(const QString& screenName)
{
    clearPerScreenSettingsImpl(screenName, &Settings::clearPerScreenAutotileSettings);
}

bool KCMAutotiling::hasPerScreenAutotileSettings(const QString& screenName) const
{
    return hasPerScreenSettingsImpl(screenName, &Settings::hasPerScreenAutotileSettings);
}

// ── Monitor disable ─────────────────────────────────────────────────────

bool KCMAutotiling::isMonitorDisabled(const QString& screenName) const
{
    return m_settings && m_settings->isMonitorDisabled(screenName);
}

void KCMAutotiling::setMonitorDisabled(const QString& screenName, bool disabled)
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

#include "kcmautotiling.moc"
