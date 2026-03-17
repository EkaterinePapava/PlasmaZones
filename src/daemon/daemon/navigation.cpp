// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../daemon.h"
#include "helpers.h"
#include "macros.h"
#include "../overlayservice.h"
#include "../unifiedlayoutcontroller.h"
#include "../../core/logging.h"
#include "../../core/utils.h"
#include "../../core/screenmanager.h"
#include "../../dbus/windowtrackingadaptor.h"
#include "../../autotile/AutotileEngine.h"
#include "../../autotile/AlgorithmRegistry.h"
#include "../../snap/SnapEngine.h"
#include "../../core/iwindowengine.h"
#include "../modetracker.h"
#include <QScreen>

namespace PlasmaZones {

// ═══════════════════════════════════════════════════════════════════════════════
// Engine routing
// ═══════════════════════════════════════════════════════════════════════════════

IWindowEngine* Daemon::engineForScreen(const QString& screenId) const
{
    if (m_autotileEngine && m_autotileEngine->isAutotileScreen(screenId)) {
        return m_autotileEngine.get();
    }
    if (m_snapEngine) {
        return m_snapEngine.get();
    }
    return nullptr;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Navigation handlers — single code path per operation (DRY/SOLID)
// Resolve screen → check mode (autotile vs zones) → delegate → OSD from backend
// ═══════════════════════════════════════════════════════════════════════════════

void Daemon::handleRotate(bool clockwise)
{
    QScreen* screen = resolveShortcutScreen(m_windowTrackingAdaptor);
    if (!screen) {
        qCDebug(lcDaemon) << "Rotate shortcut: no screen info";
        return;
    }
    QString screenId = Utils::screenIdentifier(screen);
    if (auto* engine = engineForScreen(screenId)) {
        engine->rotateWindows(clockwise, screenId);
    }
}

void Daemon::handleFloat()
{
    // Delegate to WTA → effect → unified toggleFloatForWindow.
    // The effect resolves the active KWin window + screen, stores both pre-snap
    // and pre-autotile geometry, then calls toggleFloatForWindow which the daemon
    // routes internally to snapping toggle or autotile engine based on screen mode.
    if (!m_windowTrackingAdaptor) {
        return;
    }
    m_windowTrackingAdaptor->toggleWindowFloat();
}

void Daemon::handleMove(NavigationDirection direction)
{
    QScreen* screen = resolveShortcutScreen(m_windowTrackingAdaptor);
    if (!screen) {
        return;
    }
    QString dirStr = navigationDirectionToString(direction);
    if (dirStr.isEmpty()) {
        qCWarning(lcDaemon) << "Unknown move navigation direction:" << static_cast<int>(direction);
        return;
    }
    QString screenId = Utils::screenIdentifier(screen);
    if (m_autotileEngine && m_autotileEngine->isAutotileScreen(screenId)) {
        m_autotileEngine->swapFocusedInDirection(dirStr);
    } else if (m_snapEngine) {
        m_snapEngine->moveInDirection(dirStr);
    }
}

void Daemon::handleFocus(NavigationDirection direction)
{
    QScreen* screen = resolveShortcutScreen(m_windowTrackingAdaptor);
    if (!screen) {
        return;
    }
    QString dirStr = navigationDirectionToString(direction);
    if (dirStr.isEmpty()) {
        qCWarning(lcDaemon) << "Unknown focus navigation direction:" << static_cast<int>(direction);
        return;
    }
    QString screenId = Utils::screenIdentifier(screen);
    if (auto* engine = engineForScreen(screenId)) {
        engine->focusInDirection(dirStr, QStringLiteral("focus"));
    }
}

void Daemon::handlePush()
{
    QScreen* screen = resolveShortcutScreen(m_windowTrackingAdaptor);
    if (!screen) {
        qCDebug(lcDaemon) << "PushToEmptyZone shortcut: no screen info";
        return;
    }
    QString screenId = Utils::screenIdentifier(screen);
    if (m_autotileEngine && m_autotileEngine->isAutotileScreen(screenId)) {
        return;
    }
    if (m_snapEngine) {
        m_snapEngine->pushToEmptyZone(screenId);
    }
}

void Daemon::handleRestore()
{
    QScreen* screen = resolveShortcutScreen(m_windowTrackingAdaptor);
    if (!screen) {
        return;
    }
    QString screenId = Utils::screenIdentifier(screen);
    if (m_autotileEngine && m_autotileEngine->isAutotileScreen(screenId)) {
        // In autotile mode, "restore" floats the window (equivalent to unsnapping)
        m_autotileEngine->toggleFocusedWindowFloat();
        return;
    }
    m_windowTrackingAdaptor->restoreWindowSize();
}

void Daemon::handleSwap(NavigationDirection direction)
{
    QScreen* screen = resolveShortcutScreen(m_windowTrackingAdaptor);
    if (!screen) {
        return;
    }
    QString dirStr = navigationDirectionToString(direction);
    if (dirStr.isEmpty()) {
        qCWarning(lcDaemon) << "Unknown swap navigation direction:" << static_cast<int>(direction);
        return;
    }
    QString screenId = Utils::screenIdentifier(screen);
    if (auto* engine = engineForScreen(screenId)) {
        engine->swapInDirection(dirStr, QStringLiteral("swap"));
    }
}

void Daemon::handleSnap(int zoneNumber)
{
    QScreen* screen = resolveShortcutScreen(m_windowTrackingAdaptor);
    if (!screen) {
        qCDebug(lcDaemon) << "SnapToZone shortcut: no screen info";
        return;
    }
    QString screenId = Utils::screenIdentifier(screen);
    if (auto* engine = engineForScreen(screenId)) {
        engine->moveToPosition(QString(), zoneNumber, screenId);
    }
}

void Daemon::handleCycle(bool forward)
{
    QScreen* screen = resolveShortcutScreen(m_windowTrackingAdaptor);
    if (!screen) {
        return;
    }
    QString screenId = Utils::screenIdentifier(screen);
    if (m_autotileEngine && m_autotileEngine->isAutotileScreen(screenId)) {
        QString dirStr = forward ? QStringLiteral("right") : QStringLiteral("left");
        m_autotileEngine->focusInDirection(dirStr, QStringLiteral("cycle"));
    } else if (m_snapEngine) {
        m_snapEngine->cycleWindowsInZone(forward);
    }
}

void Daemon::handleResnap()
{
    QScreen* screen = resolveShortcutScreen(m_windowTrackingAdaptor);
    if (!screen) {
        return;
    }
    QString screenId = Utils::screenIdentifier(screen);
    if (m_autotileEngine && m_autotileEngine->isAutotileScreen(screenId)) {
        m_autotileEngine->retile(screenId);
    } else if (m_snapEngine) {
        m_snapEngine->resnapToNewLayout();
    }
}

void Daemon::handleSnapAll()
{
    QScreen* screen = resolveShortcutScreen(m_windowTrackingAdaptor);
    if (!screen) {
        qCDebug(lcDaemon) << "SnapAllWindows shortcut: no screen info";
        return;
    }
    QString screenId = Utils::screenIdentifier(screen);
    if (m_autotileEngine && m_autotileEngine->isAutotileScreen(screenId)) {
        m_autotileEngine->retile(screenId);
    } else if (m_snapEngine) {
        m_snapEngine->snapAllWindows(screenId);
    }
}

// DRY macro invocations for identical autotile-only handlers
HANDLE_AUTOTILE_ONLY(FocusMaster, focusMaster())
HANDLE_AUTOTILE_ONLY(SwapWithMaster, swapFocusedWithMaster())
HANDLE_AUTOTILE_ONLY(IncreaseMasterRatio, increaseMasterRatio())
HANDLE_AUTOTILE_ONLY(DecreaseMasterRatio, decreaseMasterRatio())
HANDLE_AUTOTILE_ONLY(IncreaseMasterCount, increaseMasterCount())
HANDLE_AUTOTILE_ONLY(DecreaseMasterCount, decreaseMasterCount())

void Daemon::handleRetile()
{
    if (!m_autotileEngine || !m_autotileEngine->isEnabled()) {
        return;
    }
    m_autotileEngine->retile();
    if (m_settings && m_settings->showNavigationOsd() && m_overlayService) {
        QScreen* screen = resolveShortcutScreen(m_windowTrackingAdaptor);
        QString screenId = screen ? Utils::screenIdentifier(screen) : QString();
        if (screenId.isEmpty() && !m_autotileEngine->autotileScreens().isEmpty()) {
            screenId = *m_autotileEngine->autotileScreens().begin();
        }
        m_overlayService->showNavigationOsd(true, QStringLiteral("retile"), QStringLiteral("retiled"), QString(),
                                            QString(), screenId);
    }
}

void Daemon::resnapIfManualMode()
{
    if (!m_snapEngine) {
        return;
    }
    // Only skip resnap when the current screen is in autotile mode.
    // Per-desktop assignments mean some screens can be autotile while
    // others are manual — a global check would block manual resnaps.
    if (m_autotileEngine && m_unifiedLayoutController) {
        const QString screenId = m_unifiedLayoutController->currentScreenName();
        if (screenId.isEmpty()) {
            return; // No screen context — can't determine mode, skip resnap
        }
        if (m_autotileEngine->isAutotileScreen(screenId)) {
            return; // This screen is autotile — engine handles retile
        }
    }
    m_suppressResnapOsd = 1;
    m_snapEngine->resnapToNewLayout();
}

} // namespace PlasmaZones
