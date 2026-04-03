// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configbackend_json.h"
#include "configdefaults.h"
#include <QColor>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

namespace PlasmaZones {

// ── JsonConfigGroup ─────────────────────────────────────────────────────────

JsonConfigGroup::JsonConfigGroup(QJsonObject& root, const QString& groupName, JsonConfigBackend* backend)
    : m_root(root)
    , m_groupName(groupName)
    , m_backend(backend)
{
    Q_ASSERT_X(m_backend->m_activeGroupCount.load(std::memory_order_relaxed) == 0, "JsonConfigGroup::JsonConfigGroup",
               "Another JsonConfigGroup is still alive — destroy it first");
    m_backend->m_activeGroupCount.fetch_add(1, std::memory_order_relaxed);
}

JsonConfigGroup::~JsonConfigGroup()
{
    m_backend->m_activeGroupCount.fetch_sub(1, std::memory_order_relaxed);
}

bool JsonConfigGroup::isPerScreenGroup() const
{
    // Only the three known per-screen prefixes are routed through PerScreen.
    // Other colon-containing groups (e.g., Assignment:ScreenId:Desktop:1)
    // are stored as regular top-level groups.
    return m_groupName.startsWith(QLatin1String("ZoneSelector:"))
        || m_groupName.startsWith(QLatin1String("AutotileScreen:"))
        || m_groupName.startsWith(QLatin1String("SnappingScreen:"));
}

JsonConfigGroup::PerScreenPath JsonConfigGroup::parsePerScreenGroup() const
{
    PerScreenPath path;
    const int colonIdx = m_groupName.indexOf(QLatin1Char(':'));
    if (colonIdx < 0) {
        return path;
    }

    const QString prefix = m_groupName.left(colonIdx);
    path.screenId = m_groupName.mid(colonIdx + 1);

    // Map old prefix names to per-screen categories
    if (prefix == QLatin1String("ZoneSelector")) {
        path.category = QStringLiteral("ZoneSelector");
    } else if (prefix == QLatin1String("AutotileScreen")) {
        path.category = QStringLiteral("Autotile");
    } else if (prefix == QLatin1String("SnappingScreen")) {
        path.category = QStringLiteral("Snapping");
    } else {
        // Unknown prefix — use as-is
        path.category = prefix;
    }
    return path;
}

QJsonObject JsonConfigGroup::groupObject() const
{
    if (isPerScreenGroup()) {
        const auto path = parsePerScreenGroup();
        const QJsonObject perScreen = m_root.value(QLatin1String(JsonConfigBackend::PerScreenKey)).toObject();
        const QJsonObject category = perScreen.value(path.category).toObject();
        return category.value(path.screenId).toObject();
    }
    return m_root.value(m_groupName).toObject();
}

void JsonConfigGroup::setGroupObject(const QJsonObject& obj)
{
    if (isPerScreenGroup()) {
        const auto path = parsePerScreenGroup();
        QJsonObject perScreen = m_root.value(QLatin1String(JsonConfigBackend::PerScreenKey)).toObject();
        QJsonObject category = perScreen.value(path.category).toObject();
        category[path.screenId] = obj;
        perScreen[path.category] = category;
        m_root[QLatin1String(JsonConfigBackend::PerScreenKey)] = perScreen;
    } else {
        m_root[m_groupName] = obj;
    }
    m_backend->markDirty();
}

QString JsonConfigGroup::readString(const QString& key, const QString& defaultValue) const
{
    const QJsonObject obj = groupObject();
    if (!obj.contains(key)) {
        return defaultValue;
    }
    const QJsonValue val = obj.value(key);
    // If the value is a JSON array or object, return compact JSON string
    // so parseTriggerListJson() and similar callers work unchanged.
    if (val.isArray()) {
        return QString::fromUtf8(QJsonDocument(val.toArray()).toJson(QJsonDocument::Compact));
    }
    if (val.isObject()) {
        return QString::fromUtf8(QJsonDocument(val.toObject()).toJson(QJsonDocument::Compact));
    }
    return val.toString(defaultValue);
}

int JsonConfigGroup::readInt(const QString& key, int defaultValue) const
{
    const QJsonObject obj = groupObject();
    if (!obj.contains(key)) {
        return defaultValue;
    }
    const QJsonValue val = obj.value(key);
    if (val.isDouble()) {
        return static_cast<int>(val.toDouble());
    }
    // Handle string values (from hand-edited configs)
    if (val.isString()) {
        bool ok = false;
        const int result = val.toString().toInt(&ok);
        return ok ? result : defaultValue;
    }
    return defaultValue;
}

bool JsonConfigGroup::readBool(const QString& key, bool defaultValue) const
{
    const QJsonObject obj = groupObject();
    if (!obj.contains(key)) {
        return defaultValue;
    }
    const QJsonValue val = obj.value(key);
    if (val.isBool()) {
        return val.toBool();
    }
    // Handle string values for compatibility
    if (val.isString()) {
        const QString s = val.toString().toLower();
        if (s == QLatin1String("true") || s == QLatin1String("1") || s == QLatin1String("yes")
            || s == QLatin1String("on")) {
            return true;
        }
        if (s == QLatin1String("false") || s == QLatin1String("0") || s == QLatin1String("no")
            || s == QLatin1String("off")) {
            return false;
        }
    }
    // Handle numeric values (1 = true, 0 = false)
    if (val.isDouble()) {
        return val.toDouble() != 0.0;
    }
    return defaultValue;
}

double JsonConfigGroup::readDouble(const QString& key, double defaultValue) const
{
    const QJsonObject obj = groupObject();
    if (!obj.contains(key)) {
        return defaultValue;
    }
    const QJsonValue val = obj.value(key);
    if (val.isDouble()) {
        return val.toDouble();
    }
    if (val.isString()) {
        bool ok = false;
        const double result = val.toString().toDouble(&ok);
        return ok ? result : defaultValue;
    }
    return defaultValue;
}

QColor JsonConfigGroup::readColor(const QString& key, const QColor& defaultValue) const
{
    const QJsonObject obj = groupObject();
    if (!obj.contains(key)) {
        return defaultValue;
    }
    const QString s = obj.value(key).toString();
    if (s.isEmpty()) {
        return defaultValue;
    }

    // Try hex format first: #rrggbb or #rrggbbaa
    if (s.startsWith(QLatin1Char('#'))) {
        QColor c(s);
        return c.isValid() ? c : defaultValue;
    }

    // KConfig comma format: r,g,b or r,g,b,a
    const QStringList parts = s.split(QLatin1Char(','));
    if (parts.size() >= 3) {
        bool okR = false, okG = false, okB = false;
        int r = parts[0].trimmed().toInt(&okR);
        int g = parts[1].trimmed().toInt(&okG);
        int b = parts[2].trimmed().toInt(&okB);
        if (okR && okG && okB) {
            int a = 255;
            if (parts.size() >= 4) {
                bool okA = false;
                a = parts[3].trimmed().toInt(&okA);
                if (!okA) {
                    a = 255;
                }
            }
            return QColor(r, g, b, a);
        }
    }
    return defaultValue;
}

void JsonConfigGroup::writeString(const QString& key, const QString& value)
{
    QJsonObject obj = groupObject();
    // If the string is JSON (array or object), store as native JSON
    if (!value.isEmpty() && (value.front() == QLatin1Char('[') || value.front() == QLatin1Char('{'))) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(value.toUtf8(), &err);
        if (err.error == QJsonParseError::NoError) {
            if (doc.isArray()) {
                obj[key] = doc.array();
            } else if (doc.isObject()) {
                obj[key] = doc.object();
            } else {
                obj[key] = value;
            }
            setGroupObject(obj);
            return;
        }
    }
    obj[key] = value;
    setGroupObject(obj);
}

void JsonConfigGroup::writeInt(const QString& key, int value)
{
    QJsonObject obj = groupObject();
    obj[key] = value;
    setGroupObject(obj);
}

void JsonConfigGroup::writeBool(const QString& key, bool value)
{
    QJsonObject obj = groupObject();
    obj[key] = value;
    setGroupObject(obj);
}

void JsonConfigGroup::writeDouble(const QString& key, double value)
{
    QJsonObject obj = groupObject();
    obj[key] = value;
    setGroupObject(obj);
}

void JsonConfigGroup::writeColor(const QString& key, const QColor& value)
{
    QJsonObject obj = groupObject();
    // Store as #rrggbbaa hex
    obj[key] = value.name(QColor::HexArgb);
    setGroupObject(obj);
}

bool JsonConfigGroup::hasKey(const QString& key) const
{
    return groupObject().contains(key);
}

void JsonConfigGroup::deleteKey(const QString& key)
{
    QJsonObject obj = groupObject();
    if (obj.contains(key)) {
        obj.remove(key);
        setGroupObject(obj);
    }
}

// ── JsonConfigBackend ───────────────────────────────────────────────────────

JsonConfigBackend::JsonConfigBackend(const QString& filePath)
    : m_filePath(filePath)
{
    loadFromDisk();
}

JsonConfigBackend::~JsonConfigBackend()
{
    Q_ASSERT_X(m_activeGroupCount.load(std::memory_order_relaxed) == 0, "JsonConfigBackend::~JsonConfigBackend",
               "JsonConfigGroup still alive when backend is being destroyed");
}

void JsonConfigBackend::loadFromDisk()
{
    QFile f(m_filePath);
    if (!f.exists()) {
        m_root = QJsonObject();
        return;
    }
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("JsonConfigBackend: failed to open %s for reading", qPrintable(m_filePath));
        m_root = QJsonObject();
        return;
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning("JsonConfigBackend: JSON parse error in %s at offset %d: %s", qPrintable(m_filePath), err.offset,
                 qPrintable(err.errorString()));
        m_root = QJsonObject();
        return;
    }
    m_root = doc.object();
}

void JsonConfigBackend::markDirty()
{
    m_dirty = true;
}

std::unique_ptr<IConfigGroup> JsonConfigBackend::group(const QString& name)
{
    return std::make_unique<JsonConfigGroup>(m_root, name, this);
}

void JsonConfigBackend::reparseConfiguration()
{
    Q_ASSERT_X(m_activeGroupCount.load(std::memory_order_relaxed) == 0, "JsonConfigBackend::reparseConfiguration",
               "Cannot reparse while JsonConfigGroup instances are alive");
    loadFromDisk();
    m_dirty = false;
}

void JsonConfigBackend::sync()
{
    if (!m_dirty) {
        return;
    }

    // Ensure parent directory exists
    QDir dir = QFileInfo(m_filePath).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(QLatin1String("."));
    }

    // Atomic write using QSaveFile (writes to temp, then renames on commit)
    QSaveFile f(m_filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning("JsonConfigBackend: failed to open %s for writing: %s", qPrintable(m_filePath),
                 qPrintable(f.errorString()));
        return;
    }

    QJsonDocument doc(m_root);
    f.write(doc.toJson(QJsonDocument::Indented));

    if (!f.commit()) {
        qWarning("JsonConfigBackend: failed to commit write to %s: %s", qPrintable(m_filePath),
                 qPrintable(f.errorString()));
        return;
    }

    m_dirty = false;
}

static bool isKnownPerScreenPrefix(const QString& name)
{
    return name.startsWith(QLatin1String("ZoneSelector:")) || name.startsWith(QLatin1String("AutotileScreen:"))
        || name.startsWith(QLatin1String("SnappingScreen:"));
}

void JsonConfigBackend::deleteGroup(const QString& name)
{
    if (isKnownPerScreenPrefix(name)) {
        const int colonIdx = name.indexOf(QLatin1Char(':'));
        const QString prefix = name.left(colonIdx);
        const QString screenId = name.mid(colonIdx + 1);

        // Map prefix to per-screen category
        QString category;
        if (prefix == QLatin1String("ZoneSelector")) {
            category = QStringLiteral("ZoneSelector");
        } else if (prefix == QLatin1String("AutotileScreen")) {
            category = QStringLiteral("Autotile");
        } else {
            category = QStringLiteral("Snapping");
        }

        QJsonObject perScreen = m_root.value(QLatin1String(PerScreenKey)).toObject();
        QJsonObject cat = perScreen.value(category).toObject();
        cat.remove(screenId);
        if (cat.isEmpty()) {
            perScreen.remove(category);
        } else {
            perScreen[category] = cat;
        }
        if (perScreen.isEmpty()) {
            m_root.remove(QLatin1String(PerScreenKey));
        } else {
            m_root[QLatin1String(PerScreenKey)] = perScreen;
        }
    } else {
        m_root.remove(name);
    }
    markDirty();
}

QString JsonConfigBackend::readRootString(const QString& key, const QString& defaultValue) const
{
    // Root-level keys live under "General" (matching QSettings INI behavior)
    const QJsonObject general = m_root.value(QStringLiteral("General")).toObject();
    if (general.contains(key)) {
        return general.value(key).toString(defaultValue);
    }
    // Also check top-level for backwards compatibility
    if (m_root.contains(key)) {
        return m_root.value(key).toString(defaultValue);
    }
    return defaultValue;
}

void JsonConfigBackend::writeRootString(const QString& key, const QString& value)
{
    QJsonObject general = m_root.value(QStringLiteral("General")).toObject();
    general[key] = value;
    m_root[QStringLiteral("General")] = general;
    markDirty();
}

void JsonConfigBackend::removeRootKey(const QString& key)
{
    QJsonObject general = m_root.value(QStringLiteral("General")).toObject();
    if (general.contains(key)) {
        general.remove(key);
        if (general.isEmpty()) {
            m_root.remove(QStringLiteral("General"));
        } else {
            m_root[QStringLiteral("General")] = general;
        }
        markDirty();
    }
    // Also remove from top-level if present
    if (m_root.contains(key)) {
        m_root.remove(key);
        markDirty();
    }
}

QStringList JsonConfigBackend::groupList() const
{
    QStringList groups;

    // Add all top-level keys that are objects (except PerScreen and _version)
    for (auto it = m_root.constBegin(); it != m_root.constEnd(); ++it) {
        if (it.key() == QLatin1String(PerScreenKey) || it.key() == QLatin1String("_version")) {
            continue;
        }
        if (it.value().isObject()) {
            groups.append(it.key());
        }
    }

    // Flatten per-screen groups into Prefix:ScreenId format
    const QJsonObject perScreen = m_root.value(QLatin1String(PerScreenKey)).toObject();
    for (auto catIt = perScreen.constBegin(); catIt != perScreen.constEnd(); ++catIt) {
        const QJsonObject category = catIt.value().toObject();
        // Reverse-map category names to the old prefix format
        QString prefix = catIt.key();
        if (prefix == QLatin1String("Autotile")) {
            prefix = QStringLiteral("AutotileScreen");
        } else if (prefix == QLatin1String("Snapping")) {
            prefix = QStringLiteral("SnappingScreen");
        }
        // ZoneSelector stays as-is

        for (auto screenIt = category.constBegin(); screenIt != category.constEnd(); ++screenIt) {
            groups.append(prefix + QLatin1Char(':') + screenIt.key());
        }
    }

    return groups;
}

// ── Static factory ──────────────────────────────────────────────────────────

std::unique_ptr<JsonConfigBackend> JsonConfigBackend::createDefault()
{
    return std::make_unique<JsonConfigBackend>(ConfigDefaults::configFilePath());
}

QMap<QString, QVariant> JsonConfigBackend::readConfigFromDisk()
{
    QMap<QString, QVariant> map;

    QFile f(ConfigDefaults::configFilePath());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return map;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        return map;
    }

    const QJsonObject root = doc.object();

    // Flatten nested JSON into "Group/Key" format
    for (auto it = root.constBegin(); it != root.constEnd(); ++it) {
        if (it.key() == QLatin1String("_version")) {
            continue;
        }
        if (it.value().isObject()) {
            if (it.key() == QLatin1String("PerScreen")) {
                // Flatten per-screen: PerScreen/Category/ScreenId/Key → Category:ScreenId/Key
                const QJsonObject perScreen = it.value().toObject();
                for (auto catIt = perScreen.constBegin(); catIt != perScreen.constEnd(); ++catIt) {
                    QString prefix = catIt.key();
                    if (prefix == QLatin1String("Autotile")) {
                        prefix = QStringLiteral("AutotileScreen");
                    } else if (prefix == QLatin1String("Snapping")) {
                        prefix = QStringLiteral("SnappingScreen");
                    }
                    const QJsonObject category = catIt.value().toObject();
                    for (auto sIt = category.constBegin(); sIt != category.constEnd(); ++sIt) {
                        const QJsonObject screenObj = sIt.value().toObject();
                        const QString groupKey = prefix + QLatin1Char(':') + sIt.key();
                        for (auto kIt = screenObj.constBegin(); kIt != screenObj.constEnd(); ++kIt) {
                            map.insert(groupKey + QLatin1Char('/') + kIt.key(), kIt.value().toVariant());
                        }
                    }
                }
            } else {
                // Regular group: Group/Key
                const QJsonObject groupObj = it.value().toObject();
                for (auto kIt = groupObj.constBegin(); kIt != groupObj.constEnd(); ++kIt) {
                    map.insert(it.key() + QLatin1Char('/') + kIt.key(), kIt.value().toVariant());
                }
            }
        } else {
            // Root-level non-object key
            map.insert(it.key(), it.value().toVariant());
        }
    }

    return map;
}

IConfigBackend* JsonConfigBackend::resolveBackend(IConfigBackend* shared, std::unique_ptr<JsonConfigBackend>& fallback)
{
    if (shared) {
        return shared;
    }
    fallback = createDefault();
    return fallback.get();
}

} // namespace PlasmaZones
