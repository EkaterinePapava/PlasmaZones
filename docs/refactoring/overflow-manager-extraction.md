# Overflow Manager Extraction

## Status: Proposed
## Priority: Medium
## Scope: AutotileEngine bounded context refactor

---

## Problem

Overflow management logic is scattered across ~10 methods in `AutotileEngine`:

| Method | Overflow responsibility |
|--------|----------------------|
| `applyTiling()` | Overflow detection + auto-float loop |
| `unfloatOverflowIfRoom()` | Auto-unfloat recovery when room opens |
| `clearOverflowStatus()` | Cleanup helper (DRY wrapper) |
| `removeWindow()` | Clear overflow on window close |
| `floatWindow()` | Clear overflow on user float |
| `unfloatWindow()` | Clear overflow on user unfloat |
| `performToggleFloat()` | Clear overflow on toggle |
| `setAutotileScreens()` | Screen-removal cleanup |
| `backfillWindows()` | Priority unfloat before backfill |
| `windowFocused()` | Cross-screen migration |

The `m_overflowWindows` QSet and its invariants are maintained by convention across all these methods. There is no single place to understand, test, or modify overflow behavior.

## Proposed Solution

Extract an `OverflowManager` class that owns the overflow set and exposes a focused API:

```cpp
class OverflowManager {
public:
    explicit OverflowManager(QObject* parent = nullptr);

    // Core operations
    void markOverflow(const QString& windowId, const QString& screenName);
    void clearOverflow(const QString& windowId);
    bool isOverflow(const QString& windowId) const;

    // Recovery: unfloat overflow windows when room opens on a screen
    // Returns list of window IDs that were unfloated
    QStringList recoverIfRoom(TilingState* state, const QString& screenName,
                              int maxWindows,
                              const QHash<QString, QString>& windowToScreen);

    // Cleanup
    void removeScreen(const QString& screenName,
                      const QHash<QString, QString>& windowToScreen);
    void migrateWindow(const QString& windowId,
                       const QString& oldScreen, const QString& newScreen);

    // Query
    QStringList overflowForScreen(const QString& screenName,
                                  const QHash<QString, QString>& windowToScreen) const;
    bool isEmpty() const;

private:
    // Per-screen sets (see per-screen optimization below)
    QHash<QString, QSet<QString>> m_overflow;
};
```

### AutotileEngine integration

AutotileEngine would hold `std::unique_ptr<OverflowManager> m_overflow` and delegate:

```cpp
// In applyTiling(), replace the overflow loop:
QStringList newlyOverflowed = m_overflow->applyOverflow(state, screenName, zones.size());
for (const QString& wid : newlyOverflowed) {
    Q_EMIT windowFloatingChanged(wid, true, screenName);
}

// In floatWindow(), unfloatWindow(), etc:
m_overflow->clearOverflow(windowId);  // replaces clearOverflowStatus()

// In windowFocused():
m_overflow->migrateWindow(windowId, oldScreen, newScreen);

// In setAutotileScreens():
m_overflow->removeScreen(removedScreen, m_windowToScreen);

// In backfillWindows():
m_overflow->recoverIfRoom(state, screenName, maxWin, m_windowToScreen);
```

## Dependencies

OverflowManager needs access to:
- `TilingState*` — to call `setFloating()`, `isFloating()`, `tiledWindowCount()`
- `m_windowToScreen` — to map windows to screens (passed as const ref)
- `effectiveMaxWindows()` — passed as int parameter

It does NOT need to emit signals directly. All signal emission stays in AutotileEngine (the caller processes the returned lists).

## Per-Screen Storage (bundled)

This refactor naturally incorporates the per-screen optimization. Instead of a global `QSet<QString>`, OverflowManager uses `QHash<QString, QSet<QString>>` keyed by screen name. This eliminates the O(N) filtering in `recoverIfRoom()` and makes screen-removal cleanup trivial.

See: [Per-Screen Overflow Optimization](#per-screen-overflow-optimization) section below.

## Testing

OverflowManager can be unit tested independently:

```cpp
void testMarkAndClear()
{
    OverflowManager mgr;
    mgr.markOverflow("win-1", "Screen1");
    QVERIFY(mgr.isOverflow("win-1"));
    mgr.clearOverflow("win-1");
    QVERIFY(!mgr.isOverflow("win-1"));
}

void testRecoverIfRoom()
{
    OverflowManager mgr;
    TilingState state;
    // ... setup overflow windows, then verify recovery
}

void testScreenRemoval()
{
    OverflowManager mgr;
    mgr.markOverflow("win-1", "Screen1");
    mgr.removeScreen("Screen1", windowToScreen);
    QVERIFY(mgr.isEmpty());
}

void testCrossScreenMigration()
{
    OverflowManager mgr;
    mgr.markOverflow("win-1", "Screen1");
    mgr.migrateWindow("win-1", "Screen1", "Screen2");
    QCOMPARE(mgr.overflowForScreen("Screen2", windowToScreen).size(), 1);
}
```

## Risks

- **Signal ordering**: AutotileEngine currently emits `windowFloatingChanged` at specific points in the retile pipeline. Extracting overflow logic means the caller must emit signals at the right time — returned lists make this explicit but require discipline.
- **TilingState coupling**: OverflowManager calls `state->setFloating()` which mutates external state. This is a necessary side effect but means OverflowManager is not purely functional.
- **Scope creep**: The extraction touches every method that handles overflow. Must be done as an isolated refactor with no feature changes, and with all existing tests passing throughout.

## Estimated Impact

- ~120 lines extracted from AutotileEngine into OverflowManager (~80 lines new class + ~40 lines delegation)
- Net ~30 line reduction in AutotileEngine (removal of inline overflow logic)
- 4-6 new unit tests for OverflowManager
- `clearOverflowStatus()` helper becomes redundant (replaced by `m_overflow->clearOverflow()`)

---

## Per-Screen Overflow Optimization

### Current Implementation

```cpp
// AutotileEngine.h
QSet<QString> m_overflowWindows;  // global set, all screens mixed together
```

Every operation that needs overflow windows for a specific screen must iterate the entire set and filter:

```cpp
// In unfloatOverflowIfRoom():
for (const QString& wid : std::as_const(m_overflowWindows)) {
    if (m_windowToScreen.value(wid) == screenName && state->isFloating(wid)) {
        overflowForScreen.append(wid);
    }
}
```

This is O(total_overflow) per screen per retile.

### Proposed Implementation

```cpp
// OverflowManager (or directly in AutotileEngine if extracted separately)
QHash<QString, QSet<QString>> m_overflow;  // screenName -> overflow window IDs
```

Operations become O(1) lookup + O(screen_overflow):

```cpp
// Direct screen access:
const QSet<QString>& onScreen = m_overflow.value(screenName);
for (const QString& wid : onScreen) { ... }

// Screen removal:
m_overflow.remove(screenName);

// Cross-screen migration:
m_overflow[oldScreen].remove(windowId);
m_overflow[newScreen].insert(windowId);
```

### Behavioral Changes

| Operation | Before (global set) | After (per-screen) |
|-----------|-------------------|-------------------|
| `unfloatOverflowIfRoom(screen)` | O(N) filter all overflow | O(1) lookup + O(k) iterate screen's set |
| `setAutotileScreens` removal | O(N) filter by screen | O(1) remove key |
| `windowFocused` migration | O(1) remove + check | O(1) remove from old + insert to new |
| `clearOverflow(windowId)` | O(1) set remove | O(1) if screen known, else O(screens) |
| `isEmpty()` | O(1) | O(screens) or cached |

### When `clearOverflow(windowId)` needs screen lookup

The one complexity: `clearOverflow()` is called from `removeWindow()`, `floatWindow()`, etc. which know the windowId but don't always pass the screen. Two options:

1. **Always pass screen**: Caller looks up `m_windowToScreen.value(windowId)` and passes it. Trivial since all callers already have access.
2. **Reverse index**: OverflowManager maintains `QHash<QString, QString> m_windowToScreen` mapping overflow windows to their screen. Adds memory but makes `clearOverflow(windowId)` O(1).

Option 1 is simpler and preferred.

### Practical Impact

With typical usage (1-3 screens, 0-5 overflow windows total), the performance difference is negligible. The real value is:

- **Eliminates a class of bugs**: Cross-screen orphan bugs become impossible — overflow tracking is inherently per-screen.
- **Simpler cleanup**: Screen removal is one line, not a filter loop.
- **Clearer invariants**: Each screen's overflow set is independent and self-contained.
