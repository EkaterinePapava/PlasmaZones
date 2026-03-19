# KDE Dependency Migration Plan

Make the PlasmaZones daemon portable across Wayland compositors while
retaining full KDE integration when running under KDE Plasma.

## Scope

This covers the **daemon** (`src/core/`, `src/daemon/`, `src/dbus/`,
`src/autotile/`, `src/config/`, `src/snap/`).  The KWin effect plugin
(`kwin-effect/`), KCM (`kcm/`), and editor (`src/editor/`) are
KDE-only by nature and are **out of scope**.

## Current State

- 200 daemon source files
- 33 files (16.5%) import KDE headers
- 167 files (83.5%) are pure Qt/C++
- Zero compile-time dependency on KWin (all KWin interaction is D-Bus)

## Strategy

Introduce a **backend abstraction** compiled via CMake options.  Each
KDE dependency gets a thin interface; the KDE implementation stays as
one backend, and a portable Qt-only implementation is added alongside
it.  A CMake option (`-DUSE_KDE_FRAMEWORKS=ON|OFF`) selects which
backend to compile.

When `USE_KDE_FRAMEWORKS=ON` (default), behavior is identical to today.
When `OFF`, the daemon builds with Qt 6 + LayerShellQt only.

---

## 1. KLocalizedString → QCoreApplication::translate()

**Files affected:** 19 (14 autotile algorithms + 5 daemon/dbus files)
**~557 call sites** across daemon + editor (daemon subset: ~80)

### Current
```cpp
#include <KLocalizedString>
i18n("Master Stack")
i18nc("context", "text")
```

### Replacement
```cpp
#include "pz_i18n.h"  // project header

// pz_i18n.h — single header, no .cpp needed
#pragma once
#ifdef USE_KDE_FRAMEWORKS
  #include <KLocalizedString>
  // i18n() and i18nc() already defined by KLocalizedString
#else
  #include <QCoreApplication>
  #define i18n(text) QCoreApplication::translate("plasmazonesd", text)
  #define i18nc(context, text) QCoreApplication::translate(context, text)
  #define i18np(singular, plural, n) \
      ((n == 1) ? QCoreApplication::translate("plasmazonesd", singular) \
                : QCoreApplication::translate("plasmazonesd", plural))
  #define ki18n(text) QString::fromUtf8(text)
#endif
```

### Translation files
- Qt uses `.ts` (source) → `.qm` (compiled) via `lupdate`/`lrelease`
- KDE uses `.po` → `.mo` via gettext
- Convert existing `.po` with: `lconvert -i messages.po -o messages.ts`
- Ship both formats; CMake selects which to install

### Migration steps
1. Create `src/pz_i18n.h` with the conditional header above
2. Replace all `#include <KLocalizedString>` / `#include <KLocalizedContext>`
   in daemon sources with `#include "pz_i18n.h"`
3. In `daemon/main.cpp`, replace `KLocalizedString::setApplicationDomain()`
   with a no-op when `USE_KDE_FRAMEWORKS` is off (Qt loads `.qm` files via
   `QTranslator::load()` instead)
4. Add `QTranslator` loading in the non-KDE main() path

### Risk: Low
Pure string wrapping. No behavioral change.

---

## 2. KConfig / KSharedConfig / KConfigGroup → QSettings

**Files affected:** 9
`config/settings.cpp`, `config/settings.h`, `config/settings/loadsave.cpp`,
`config/settings/perscreen.cpp`, `core/layoutmanager/persistence.cpp`,
`core/shaderregistry/params.cpp`, `autotile/SettingsBridge.cpp`,
`dbus/windowtrackingadaptor/saveload.cpp`,
`dbus/windowtrackingadaptor/persistence.cpp`

### Current
```cpp
KSharedConfigPtr config = KSharedConfig::openConfig(QStringLiteral("plasmazonesrc"));
KConfigGroup group = config->group("General");
int value = group.readEntry("Key", 42);
group.writeEntry("Key", value);
config->sync();
```

### Replacement
Introduce `src/config/configbackend.h`:

```cpp
#pragma once
#include <QString>
#include <QVariant>

namespace PlasmaZones {

class IConfigBackend {
public:
    virtual ~IConfigBackend() = default;
    virtual QVariant read(const QString& group, const QString& key,
                          const QVariant& defaultValue) = 0;
    virtual void write(const QString& group, const QString& key,
                       const QVariant& value) = 0;
    virtual void sync() = 0;
    virtual void deleteGroup(const QString& group) = 0;
    virtual QStringList groupList() = 0;
    virtual QStringList keyList(const QString& group) = 0;
};

} // namespace PlasmaZones
```

**KDE backend** (`configbackend_kconfig.cpp`):
Wraps `KSharedConfig::openConfig("plasmazonesrc")` — identical to today.

**Qt backend** (`configbackend_qsettings.cpp`):
Uses `QSettings` in INI format reading/writing the same
`~/.config/plasmazonesrc` file.  KConfig's on-disk format is standard
INI (`[Group]` / `Key=Value`), and `QSettings::IniFormat` is compatible.

```cpp
QSettings settings(QDir::homePath() + "/.config/plasmazonesrc",
                   QSettings::IniFormat);
```

### KConfigXT / .kcfg
The `plasmazones.kcfg` schema and the generated `PlasmaZonesConfig` class
are KDE-only.  The `configdefaults.h` wrapper already reads defaults from
the generated class.  For the Qt backend, embed the same defaults as
compile-time constants (they're already listed in `configdefaults.h`).

### Config file
Both backends read and write **`~/.config/plasmazonesrc`**.  No migration
needed — users can switch between KDE and non-KDE builds without losing
settings.

Known compatibility notes:
- KConfig supports `[$d]` (default markers) and `[$i]` (immutable) —
  `QSettings` will preserve these as literal key suffixes (harmless,
  they just won't be interpreted as directives)
- KConfig list separator is `,` by default; ensure `QSettings` uses
  the same when reading list values

### Migration steps
1. Create `IConfigBackend` interface
2. Implement `KConfigBackend` wrapping current KConfig calls
3. Implement `QSettingsBackend` using `QSettings`
4. Refactor `Settings` class to accept `IConfigBackend*` (dependency injection)
5. Refactor persistence files to use the backend interface
6. CMake selects implementation at compile time

### Risk: Medium
Settings is deeply integrated. This is the largest single migration item.
The interface boundary is clear (read/write/sync/groups), but thorough
testing of all settings paths is required.

---

## 3. KGlobalAccel → XDG Desktop Portal GlobalShortcuts

**Files affected:** 4
`daemon/shortcutmanager.cpp`, `daemon/shortcutmanager/handlers.cpp`,
`dbus/windowdragadaptor.cpp`, `dbus/windowdragadaptor/drag.cpp`

### Current
```cpp
#include <KGlobalAccel>
KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::META | Qt::Key_E));
KGlobalAccel::self()->removeAllShortcuts(action);
```

### Replacement
Introduce `src/daemon/shortcutbackend.h`:

```cpp
#pragma once
#include <QObject>
#include <QKeySequence>
#include <functional>

namespace PlasmaZones {

class IShortcutBackend : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~IShortcutBackend() = default;

    /// Register a shortcut. id is stable across sessions.
    /// preferredTrigger is a hint; the compositor may assign differently.
    virtual void registerShortcut(const QString& id,
                                  const QString& description,
                                  const QKeySequence& preferredTrigger,
                                  std::function<void()> callback) = 0;
    virtual void unregisterShortcut(const QString& id) = 0;
    virtual void unregisterAll() = 0;

Q_SIGNALS:
    void shortcutActivated(const QString& id);
};

} // namespace PlasmaZones
```

**KDE backend** (`shortcutbackend_kglobalaccel.cpp`):
Wraps `KGlobalAccel::setGlobalShortcut()` — identical to today.

**Portal backend** (`shortcutbackend_portal.cpp`):
Calls `org.freedesktop.portal.GlobalShortcuts` via `QDBusInterface`:
1. `CreateSession()` on startup
2. `BindShortcuts()` for each registered shortcut
3. Listens for `Activated` / `Deactivated` signals
4. `ConfigureShortcuts()` exposed for settings UI

### Compositor support for the portal

| Compositor | Portal works? | Notes |
|------------|--------------|-------|
| KDE/KWin | Yes | Full support, reference implementation |
| GNOME | Yes | GNOME 48+ (March 2025) |
| Hyprland | Yes | Works, no GUI remap UI |
| Sway/wlroots | **No** | Blocked on `ext-` Wayland protocol |
| COSMIC | **No** | On roadmap, not shipped |

### Graceful degradation
When the portal is unavailable (Sway, COSMIC, older GNOME):
- Log a warning at startup
- Expose a D-Bus method `TriggerAction(QString actionId)` so users can
  bind compositor-native shortcuts to:
  `dbus-send --session --dest=org.plasmazones.daemon /Daemon
   org.plasmazones.daemon.TriggerAction string:"toggle-autotile"`
- Document per-compositor keybinding examples:
  - Sway: `bindsym $mod+e exec dbus-send ...`
  - Hyprland: `bind = SUPER, E, exec, dbus-send ...`

### The Escape key (WindowDragAdaptor)
`windowdragadaptor.cpp` registers/unregisters a temporary Escape shortcut
during drag operations via `KGlobalAccel`.  The portal equivalent:
- `BindShortcuts()` with Escape when drag starts
- `UnbindShortcuts()` (destroy session or rebind without it) when drag ends
- Alternatively, handle Escape via the overlay window's key event (it has
  focus during drag), bypassing global shortcuts entirely

### Migration steps
1. Create `IShortcutBackend` interface
2. Implement `KGlobalAccelBackend` wrapping current code
3. Implement `PortalShortcutBackend` using `QDBusInterface`
4. Refactor `ShortcutManager` to accept `IShortcutBackend*`
5. Refactor `WindowDragAdaptor` Escape handling
6. Add D-Bus `TriggerAction` fallback method for compositors without portal
7. CMake selects implementation; portal backend has no KDE dependency

### Risk: Medium-High
ShortcutManager has 40+ actions with deferred async registration.
The portal's "compositor assigns the actual key" model is fundamentally
different from KGlobalAccel's "app sets the key" model.  The KCM
shortcut configuration UI will need portal-aware logic (call
`ConfigureShortcuts()` to open the compositor's remap dialog).

---

## 4. KColorScheme → QPalette

**Files affected:** 3
`config/settings.cpp`, `daemon/overlayservice/overlay_data.cpp`,
`daemon/overlayservice/shader.cpp`

### Current
```cpp
#include <KColorScheme>
KColorScheme scheme(QPalette::Active, KColorScheme::View);
QColor bg = scheme.background(KColorScheme::NormalBackground).color();
QColor fg = scheme.foreground(KColorScheme::NormalText).color();
```

### Replacement
```cpp
#ifdef USE_KDE_FRAMEWORKS
  #include <KColorScheme>
  // ... existing KColorScheme code
#else
  QPalette pal = QGuiApplication::palette();
  QColor bg = pal.color(QPalette::Active, QPalette::Base);
  QColor fg = pal.color(QPalette::Active, QPalette::Text);
  QColor highlight = pal.color(QPalette::Active, QPalette::Highlight);
  QColor border = fg;
  border.setAlphaF(0.3);
#endif
```

`QPalette` respects `QT_QPA_PLATFORMTHEME`.  On non-KDE desktops, Qt
reads the platform theme (e.g., `qt6ct`, `gnome`, `lxqt`) to populate
the palette.  Colors may differ slightly from KColorScheme but will match
the user's configured Qt theme.

### Migration steps
1. Add `#ifdef` blocks in the 3 affected files
2. Map `KColorScheme::Selection/NormalBackground` → `QPalette::Highlight`
3. Map `KColorScheme::View/NormalText` → `QPalette::Text`
4. Map `KColorScheme::View/NormalBackground` → `QPalette::Base`

### Risk: Low
Three files, straightforward color mapping.

---

## 5. KDBusService → QDBusConnection

**Files affected:** 1 (`daemon/main.cpp`)

### Current
```cpp
#include <KDBusService>
KDBusService service(KDBusService::Unique | KDBusService::Replace);
connect(&service, &KDBusService::activateRequested, ...);
```

### Replacement
```cpp
#ifdef USE_KDE_FRAMEWORKS
  #include <KDBusService>
  KDBusService service(KDBusService::Unique);
  // ... existing code
#else
  QDBusConnection bus = QDBusConnection::sessionBus();
  if (!bus.registerService(QStringLiteral("org.plasmazones.daemon"))) {
      // Another instance is running
      if (parser.isSet(replaceOption)) {
          // Ask existing instance to quit, then re-register
          QDBusInterface existing(QStringLiteral("org.plasmazones.daemon"),
                                  QStringLiteral("/Daemon"),
                                  QStringLiteral("org.plasmazones.daemon"));
          existing.call(QStringLiteral("Quit"));
          QThread::msleep(500);
          bus.registerService(QStringLiteral("org.plasmazones.daemon"));
      } else {
          qCWarning(lcDaemon) << "Already running";
          return 0;
      }
  }
  bus.registerObject(QStringLiteral("/Daemon"), &daemon);
#endif
```

### Migration steps
1. Add `#ifdef` in `daemon/main.cpp`
2. Implement D-Bus service registration with `QDBusConnection`
3. Handle `--replace` flag via D-Bus call to existing instance

### Risk: Low
Single file, well-understood Qt D-Bus API.

---

## 6. KAboutData → QCoreApplication

**Files affected:** 1 (`daemon/main.cpp`)

### Current
```cpp
#include <KAboutData>
KAboutData aboutData(...);
KAboutData::setApplicationData(aboutData);
aboutData.setupCommandLine(&parser);
```

### Replacement
```cpp
#ifdef USE_KDE_FRAMEWORKS
  // ... existing KAboutData code
#else
  app.setApplicationName(QStringLiteral("plasmazonesd"));
  app.setApplicationVersion(PlasmaZones::VERSION_STRING);
  app.setOrganizationName(QStringLiteral("plasmazones"));
  app.setOrganizationDomain(QStringLiteral("org.plasmazones"));
  // --version and --help are provided by QCommandLineParser
  parser.setApplicationDescription(
      QCoreApplication::translate("main", "Window tiling and zone management"));
  parser.addHelpOption();
  parser.addVersionOption();
#endif
```

### Risk: Trivial

---

## 7. PlasmaActivities → disabled

**Files affected:** 1 (`core/activitymanager.cpp`)

Already optional, gated by `#ifdef HAVE_KACTIVITIES`.  When
`USE_KDE_FRAMEWORKS=OFF`, simply don't define `HAVE_KACTIVITIES`.
The activity manager gracefully degrades to treating all windows
as belonging to a single activity.

### Risk: None
Already implemented.

---

## CMake Changes

### Top-level CMakeLists.txt

```cmake
option(USE_KDE_FRAMEWORKS "Build with KDE Frameworks integration" ON)

if(USE_KDE_FRAMEWORKS)
    find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
    find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
        Config ConfigWidgets CoreAddons DBusAddons
        I18n KCMUtils WindowSystem GlobalAccel
        Notifications ColorScheme
    )
    # ... PlasmaActivities (optional, existing logic)
    add_compile_definitions(USE_KDE_FRAMEWORKS)
else()
    # Minimal: Qt6 + LayerShellQt only
    # ECM is still useful for install dirs, but not required
    find_package(ECM QUIET NO_MODULE)
endif()

# Always required regardless of backend
find_package(LayerShellQt 6.6 REQUIRED)
```

### src/CMakeLists.txt

```cmake
if(USE_KDE_FRAMEWORKS)
    target_sources(plasmazones_core PRIVATE
        config/configbackend_kconfig.cpp
        daemon/shortcutbackend_kglobalaccel.cpp
    )
    target_link_libraries(plasmazones_core PUBLIC
        KF6::ConfigCore KF6::ConfigGui KF6::CoreAddons
        KF6::GlobalAccel KF6::WindowSystem KF6::I18n
        KF6::ColorScheme
    )
else()
    target_sources(plasmazones_core PRIVATE
        config/configbackend_qsettings.cpp
        daemon/shortcutbackend_portal.cpp
    )
    # No KDE link dependencies
endif()
```

---

## Build Matrix

| Configuration | Dependencies | Compositors |
|--------------|-------------|-------------|
| `USE_KDE_FRAMEWORKS=ON` (default) | Qt6, KF6, LayerShellQt, ECM | KDE Plasma (full features) |
| `USE_KDE_FRAMEWORKS=OFF` | Qt6, LayerShellQt | Any Wayland compositor with LayerShell |

---

## Migration Order

Suggested implementation sequence, ordered by risk and dependency:

| Phase | Item | Risk | Files | Depends on |
|-------|------|------|-------|------------|
| 1 | pz_i18n.h (KLocalizedString) | Low | 19 | — |
| 2 | KAboutData | Trivial | 1 | — |
| 3 | KDBusService | Low | 1 | — |
| 4 | KColorScheme | Low | 3 | — |
| 5 | PlasmaActivities | None | 0 | — |
| 6 | KConfig → IConfigBackend | Medium | 9+ | — |
| 7 | KGlobalAccel → IShortcutBackend | Medium-High | 4 | Phase 6 (shortcuts stored in config) |
| 8 | CMake restructure | Low | 2 | All above |

Phases 1–5 can be done in parallel.  Phase 6 and 7 are the bulk of the
work.  Phase 8 ties it together.

---

## What Stays KDE-Only

These components are inherently KDE-specific and are not migrated:

- **KWin effect plugin** (`kwin-effect/`) — KWin C++ plugin API
- **KCM settings modules** (`kcm/`) — KDE System Settings plugin
- **ECM install macros** — `KDE_INSTALL_BINDIR`, `KDE_INSTALL_SYSTEMDUSERUNITDIR`
  (provide fallback paths when ECM is absent)
- **KConfigXT schema** (`plasmazones.kcfg`) — only used when KDE backend active
- **KNotifications** — only linked by daemon executable, not core; can be
  `#ifdef`'d (notifications are non-critical)

---

## Testing Strategy

1. CI builds both `USE_KDE_FRAMEWORKS=ON` and `OFF`
2. Existing 49 unit tests must pass in both configurations
3. Add backend-specific integration tests for:
   - Config read/write/sync round-trip (QSettings vs KConfig)
   - Shortcut registration/activation (mock portal D-Bus service)
4. Manual testing on at least: KDE Plasma, Hyprland, Sway (degraded mode)
