// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KQuickConfigModule>
#include <QString>

namespace PlasmaZones {

class UpdateChecker;

/**
 * @brief About sub-KCM — version info, update checks, links, settings launcher
 */
class KCMAbout : public KQuickConfigModule
{
    Q_OBJECT

    // Update checker
    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(bool checkingForUpdates READ checkingForUpdates NOTIFY checkingForUpdatesChanged)
    Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY updateAvailableChanged)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY latestVersionChanged)
    Q_PROPERTY(QString dismissedUpdateVersion READ dismissedUpdateVersion WRITE setDismissedUpdateVersion NOTIFY
                   dismissedUpdateVersionChanged)

public:
    KCMAbout(QObject* parent, const KPluginMetaData& data);
    ~KCMAbout() override;

    // Update checker
    QString currentVersion() const;
    bool checkingForUpdates() const;
    bool updateAvailable() const;
    QString latestVersion() const;
    QString dismissedUpdateVersion() const;
    void setDismissedUpdateVersion(const QString& version);

    Q_INVOKABLE void checkForUpdates();
    Q_INVOKABLE void openReleaseUrl();
    Q_INVOKABLE void openSettings();

Q_SIGNALS:
    void checkingForUpdatesChanged();
    void updateAvailableChanged();
    void latestVersionChanged();
    void dismissedUpdateVersionChanged();
    void releaseUrlChanged();

private:
    UpdateChecker* m_updateChecker = nullptr;
    QString m_dismissedUpdateVersion;
};

} // namespace PlasmaZones
