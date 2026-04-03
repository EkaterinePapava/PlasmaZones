// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "plasmazones_export.h"
#include <QColor>
#include <QString>
#include <QStringList>
#include <memory>

namespace PlasmaZones {

/// Abstract interface for a scoped view into a single config group.
///
/// Returned by IConfigBackend::group() as a unique_ptr.  Implementations
/// may restrict one active group per backend at a time.
class PLASMAZONES_EXPORT IConfigGroup
{
public:
    virtual ~IConfigGroup() = default;

    // Typed reads with defaults
    virtual QString readString(const QString& key, const QString& defaultValue = {}) const = 0;
    virtual int readInt(const QString& key, int defaultValue = 0) const = 0;
    virtual bool readBool(const QString& key, bool defaultValue = false) const = 0;
    virtual double readDouble(const QString& key, double defaultValue = 0.0) const = 0;
    virtual QColor readColor(const QString& key, const QColor& defaultValue = {}) const = 0;

    // Typed writes
    virtual void writeString(const QString& key, const QString& value) = 0;
    virtual void writeInt(const QString& key, int value) = 0;
    virtual void writeBool(const QString& key, bool value) = 0;
    virtual void writeDouble(const QString& key, double value) = 0;
    virtual void writeColor(const QString& key, const QColor& value) = 0;

    // Key management
    virtual bool hasKey(const QString& key) const = 0;
    virtual void deleteKey(const QString& key) = 0;

    IConfigGroup(const IConfigGroup&) = delete;
    IConfigGroup& operator=(const IConfigGroup&) = delete;

protected:
    IConfigGroup() = default;
};

/// Abstract interface for a pluggable config backend.
///
/// Provides group-based access, persistence, and enumeration.
/// Concrete implementations: QSettingsConfigBackend (INI), JsonConfigBackend (JSON).
class PLASMAZONES_EXPORT IConfigBackend
{
public:
    virtual ~IConfigBackend() = default;

    /// Get a group view.  Caller owns the returned pointer.
    virtual std::unique_ptr<IConfigGroup> group(const QString& name) = 0;

    /// Re-read config from disk (discard in-memory changes).
    virtual void reparseConfiguration() = 0;

    /// Flush pending writes to disk.
    virtual void sync() = 0;

    /// Delete an entire group and its keys.
    virtual void deleteGroup(const QString& name) = 0;

    /// Read/write ungrouped (root-level) keys.
    virtual QString readRootString(const QString& key, const QString& defaultValue = {}) const = 0;
    virtual void writeRootString(const QString& key, const QString& value) = 0;
    virtual void removeRootKey(const QString& key) = 0;

    /// List all top-level group names.
    virtual QStringList groupList() const = 0;

    IConfigBackend(const IConfigBackend&) = delete;
    IConfigBackend& operator=(const IConfigBackend&) = delete;

protected:
    IConfigBackend() = default;
};

/// Resolve a shared or fallback backend.  If @p shared is non-null it is
/// returned directly; otherwise a new default backend of type T is created
/// into @p fallback and returned.  Eliminates repeated resolve boilerplate
/// across JsonConfigBackend and QSettingsConfigBackend.
template<typename T>
IConfigBackend* resolveBackend(IConfigBackend* shared, std::unique_ptr<T>& fallback)
{
    if (shared) {
        return shared;
    }
    fallback = T::createDefault();
    return fallback.get();
}

} // namespace PlasmaZones
