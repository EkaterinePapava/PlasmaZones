// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QByteArray>

namespace PlasmaZones {

/// Call before QGuiApplication to register the pz-layer-shell QPA plugin.
/// Lightweight header — does not pull in Qt Wayland private headers.
inline void registerLayerShellPlugin()
{
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "pz-layer-shell");
}

} // namespace PlasmaZones
