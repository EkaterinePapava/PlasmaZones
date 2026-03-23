// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "plasmazones_export.h"
#include <QObject>
#include <QRect>
#include <QString>
#include <QStringList>

namespace PlasmaZones {

/**
 * @brief Abstract interface for compositor bridges
 *
 * Defines the command protocol from daemon to compositor. The KWin effect
 * implements this implicitly via D-Bus signal subscriptions; other compositor
 * bridges (Hyprland, Sway) would implement it via their native IPC.
 *
 * The daemon calls these methods to manipulate windows. The bridge translates
 * them to compositor-specific API calls.
 *
 * Window IDs use the format "appId|bridgeHandle" where bridgeHandle is an
 * opaque compositor-specific token passed back verbatim.
 *
 * @see DbusCompositorBridge, CompositorBridgeAdaptor
 */
class PLASMAZONES_EXPORT ICompositorBridge : public QObject
{
    Q_OBJECT

public:
    explicit ICompositorBridge(QObject* parent = nullptr)
        : QObject(parent)
    {
    }
    ~ICompositorBridge() override = default;

    // ═══════════════════════════════════════════════════════════════════════════
    // Identity
    // ═══════════════════════════════════════════════════════════════════════════

    /// Compositor name (e.g. "kwin", "hyprland", "sway")
    virtual QString compositorName() const = 0;

    /// Supported capabilities (e.g. "borderless", "maximize", "animation")
    virtual QStringList capabilities() const = 0;

    /// Whether a bridge is currently connected and responsive
    virtual bool isConnected() const = 0;

    // ═══════════════════════════════════════════════════════════════════════════
    // Window geometry commands
    // ═══════════════════════════════════════════════════════════════════════════

    /// Apply geometry to a single window
    virtual void applyWindowGeometry(const QString& windowId, const QRect& geometry, const QString& zoneId,
                                     bool skipAnimation = false) = 0;

    /// Apply geometry to a batch of windows (for rotate/resnap)
    virtual void applyWindowGeometriesBatch(const QString& batchJson, const QString& action) = 0;

    // ═══════════════════════════════════════════════════════════════════════════
    // Window focus and stacking
    // ═══════════════════════════════════════════════════════════════════════════

    /// Activate (focus) a window
    virtual void activateWindow(const QString& windowId) = 0;

    /// Raise windows in order (bottom-to-top)
    virtual void raiseWindows(const QStringList& windowIds) = 0;

    // ═══════════════════════════════════════════════════════════════════════════
    // Window decoration
    // ═══════════════════════════════════════════════════════════════════════════

    /// Set window borderless state (hide/show title bar)
    virtual void setWindowBorderless(const QString& windowId, bool borderless) = 0;

    /// Maximize or restore a window (0=restore, 1=vertical, 2=horizontal, 3=full)
    virtual void maximizeWindow(const QString& windowId, int mode) = 0;
};

} // namespace PlasmaZones
