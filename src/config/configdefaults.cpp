// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configdefaults.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace PlasmaZones {

static QString configDir()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    if (dir.isEmpty()) {
        dir = QDir::homePath() + QStringLiteral("/.config");
    }
    return dir;
}

QString ConfigDefaults::configFilePath()
{
    return configDir() + QStringLiteral("/plasmazones/config.json");
}

QString ConfigDefaults::legacyConfigFilePath()
{
    return configDir() + QStringLiteral("/plasmazonesrc");
}

QString ConfigDefaults::readRenderingBackendFromDisk()
{
    const QString jsonPath = configFilePath();
    QFile jsonFile(jsonPath);
    if (jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(jsonFile.readAll(), &err);
        if (err.error == QJsonParseError::NoError) {
            const QJsonObject root = doc.object();
            const QJsonObject rendering = root.value(renderingGroup()).toObject();
            if (rendering.contains(renderingBackendKey())) {
                return normalizeRenderingBackend(rendering.value(renderingBackendKey()).toString());
            }
        }
    }

    return renderingBackend();
}

} // namespace PlasmaZones
