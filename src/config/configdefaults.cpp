// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configdefaults.h"
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

namespace PlasmaZones {

QString ConfigDefaults::configFilePath()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    if (configDir.isEmpty()) {
        // QStandardPaths failed — QDir::homePath() returns "/" when $HOME is unset,
        // so the concatenation is always non-empty.
        configDir = QDir::homePath() + QStringLiteral("/.config");
    }
    return configDir + QStringLiteral("/plasmazonesrc");
}

QString ConfigDefaults::readRenderingBackendFromDisk()
{
    QSettings cfg(configFilePath(), QSettings::IniFormat);

    // Try ungrouped root first (settings app writes RenderingBackend before any [Section] header)
    QString raw = cfg.value(renderingBackendKey()).toString();
    if (raw.isEmpty()) {
        // Fallback: check [General] group
        cfg.beginGroup(generalGroup());
        raw = cfg.value(renderingBackendKey(), renderingBackend()).toString();
        cfg.endGroup();
    }
    return normalizeRenderingBackend(raw);
}

} // namespace PlasmaZones
