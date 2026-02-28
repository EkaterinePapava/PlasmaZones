# OverflowManager Extraction — Implementation Plan

## Status: Ready for Implementation
## Derived from: `docs/refactoring/overflow-manager-extraction.md`
## Branch: `feature/autotiling`

---

## Overview

Extract overflow management from AutotileEngine into a dedicated `OverflowManager` class with per-screen storage, then backfill 8 identified test coverage gaps. The refactor touches 10 methods across 3 files but introduces zero behavioral changes — pure structural extraction.

---

## Phase 1: Create OverflowManager Class

### Issue 1.1 — Create `src/autotile/OverflowManager.h`

**New file.** Owns per-screen overflow tracking with a focused API.

```cpp
class OverflowManager {
public:
    explicit OverflowManager(QObject* parent = nullptr);

    // Core operations
    void markOverflow(const QString& windowId, const QString& screenName);
    void clearOverflow(const QString& windowId, const QString& screenName);
    bool isOverflow(const QString& windowId) const;

    // Recovery: unfloat overflow windows when room opens
    // Returns list of window IDs that were unfloated (caller emits signals)
    QStringList recoverIfRoom(TilingState* state, const QString& screenName, int maxWindows);

    // Bulk overflow detection: auto-float windows beyond tileCount
    // Returns list of newly overflowed window IDs (caller emits signals)
    QStringList applyOverflow(TilingState* state, const QString& screenName,
                              const QStringList& windows, int tileCount);

    // Cleanup
    void removeWindow(const QString& windowId, const QString& screenName);
    void removeScreen(const QString& screenName);
    void migrateWindow(const QString& windowId,
                       const QString& oldScreen, const QString& newScreen);

    // Screen deactivation: returns overflow window IDs for the screen
    // (so caller can exclude them from savedFloatingWindows)
    QSet<QString> takeOverflowForScreen(const QString& screenName);

    // Query
    bool isEmpty() const;
    bool hasOverflowOnScreen(const QString& screenName) const;

private:
    QHash<QString, QSet<QString>> m_overflow;  // screenName -> overflow window IDs
    QHash<QString, QString> m_windowToScreen;  // reverse index for clearOverflow by windowId
};
```

**Key design decisions:**
- Per-screen `QHash<QString, QSet<QString>>` replaces global `QSet<QString>` (O(1) screen lookup)
- Reverse index `m_windowToScreen` makes `clearOverflow(windowId, screenName)` O(1) even without caller passing screen
- Returns lists instead of emitting signals (signal emission stays in AutotileEngine)
- `applyOverflow()` encapsulates the detection loop from `applyTiling()`
- `takeOverflowForScreen()` supports `setAutotileScreens()` deactivation flow

### Issue 1.2 — Create `src/autotile/OverflowManager.cpp`

**New file.** ~80 lines implementing the API above.

**Implementation notes:**
- `recoverIfRoom()` iterates only the screen's overflow set (not global), collects unfloatable windows, calls `state->setFloating(wid, false)` in batch, then returns the list
- `applyOverflow()` iterates `windows[tileCount..]`, skips already-overflowed, calls `state->setFloating(wid, true)`, tracks in per-screen set
- `migrateWindow()` removes from old screen set, inserts into new screen set, updates reverse index
- `removeScreen()` is a single `m_overflow.remove(screenName)` + reverse index cleanup
- All methods log via `qCInfo(lcAutotile)` / `qCDebug(lcAutotile)` matching current patterns

### Issue 1.3 — Add to CMakeLists.txt

Add `src/autotile/OverflowManager.cpp` and `src/autotile/OverflowManager.h` to the daemon target's source list.

---

## Phase 2: Integrate into AutotileEngine

### Issue 2.1 — Replace `m_overflowWindows` with `OverflowManager`

**File:** `src/autotile/AutotileEngine.h`

- Remove: `QSet<QString> m_overflowWindows` (line 812-815)
- Remove: `void unfloatOverflowIfRoom(const QString& screenName)` declaration
- Remove: `void clearOverflowStatus(const QString& windowId)` declaration
- Add: `#include "OverflowManager.h"`
- Add: `std::unique_ptr<OverflowManager> m_overflow;`
- Initialize in constructor

### Issue 2.2 — Rewire `applyTiling()` overflow detection

**File:** `src/autotile/AutotileEngine.cpp` (lines 1779-1799)

Replace inline overflow detection loop with:
```cpp
QStringList newlyOverflowed = m_overflow->applyOverflow(state, screenName, windows, tileCount);
for (const QString& wid : std::as_const(newlyOverflowed)) {
    Q_EMIT windowFloatingChanged(wid, true, screenName);
}
```

### Issue 2.3 — Rewire `unfloatOverflowIfRoom()` call sites

**File:** `src/autotile/AutotileEngine.cpp`

Replace all 5 call sites (lines 903, 913, 1920, 2010, 2021) with:
```cpp
QStringList unfloated = m_overflow->recoverIfRoom(state, screenName, effectiveMaxWindows(screenName));
for (const QString& wid : unfloated) {
    Q_EMIT windowFloatingChanged(wid, false, screenName);
}
```

Extract a private helper to avoid repeating the emit loop:
```cpp
void AutotileEngine::emitOverflowRecovery(const QString& screenName) {
    TilingState* state = stateForScreen(screenName);
    if (!state) return;
    QStringList unfloated = m_overflow->recoverIfRoom(state, screenName, effectiveMaxWindows(screenName));
    for (const QString& wid : unfloated) {
        Q_EMIT windowFloatingChanged(wid, false, screenName);
    }
}
```

### Issue 2.4 — Rewire `clearOverflowStatus()` call sites

**File:** `src/autotile/AutotileEngine.cpp`

Replace all 5 call sites (lines 1356, 1387, 1413, 1506, 1663):

| Call site | Replacement |
|-----------|-------------|
| `performToggleFloat()` | `m_overflow->clearOverflow(windowId, m_windowToScreen.value(windowId))` |
| `floatWindow()` | `m_overflow->clearOverflow(windowId, m_windowToScreen.value(windowId))` |
| `unfloatWindow()` | `m_overflow->clearOverflow(windowId, m_windowToScreen.value(windowId))` |
| `windowFocused()` migration | `m_overflow->migrateWindow(windowId, oldScreen, screenName)` (replaces clearOverflow + manual cleanup) |
| `removeWindow()` | `m_overflow->removeWindow(windowId, screenName)` |

Remove: `clearOverflowStatus()` method definition entirely.
Remove: `unfloatOverflowIfRoom()` method definition entirely.

### Issue 2.5 — Rewire `setAutotileScreens()` cleanup

**File:** `src/autotile/AutotileEngine.cpp` (lines 155-187)

Replace the two-part overflow cleanup:

```cpp
// Part 1: In the floating-windows loop (line 155-164)
const QStringList floated = it.value()->floatingWindows();
QSet<QString> screenOverflow = m_overflow->takeOverflowForScreen(it.key());
for (const QString& fid : floated) {
    if (!screenOverflow.contains(fid)) {
        m_savedFloatingWindows.insert(fid);
    }
}

// Part 2: Remove entire second cleanup loop (lines 181-187) — no longer needed
// takeOverflowForScreen() already removed all entries for the screen
```

### Issue 2.6 — Rewire `backfillWindows()` priority call

**File:** `src/autotile/AutotileEngine.cpp` (line 1920)

Replace `unfloatOverflowIfRoom(screenName)` with `emitOverflowRecovery(screenName)` (uses the helper from Issue 2.3).

---

## Phase 3: Test Coverage — Fill Identified Gaps

All new tests go in `tests/unit/test_autotile_engine.cpp`. Total: 8 new test methods.

### Issue 3.1 — `testOverflow_crossScreenMigration()` [HIGH]

**Scenario:** Overflow window moves between screens via `windowFocused()`.

- Open win-1 on ScreenA with maxWindows=10
- Open win-2 on ScreenA, trigger overflow with maxWindows=1
- Call `windowFocused("win-2", "ScreenB")`
- Verify win-2 is removed from ScreenA's TilingState
- Verify win-2 is re-added to ScreenB via `onWindowAdded()`
- Verify overflow tracking is cleared (it re-enters normal flow on ScreenB)

### Issue 3.2 — `testOverflow_backfillPriority()` [HIGH]

**Scenario:** Overflow windows recover before new windows get backfilled.

- Open win-1, win-2, win-3 on screen with maxWindows=2 (win-3 overflows)
- Open win-4 which goes to a non-autotile holding area
- Increase maxWindows to 3 and set 3 zones
- Call `retile()`
- Verify win-3 (overflow) returns to tiling, not win-4

### Issue 3.3 — `testOverflow_multipleUnfloat()` [MEDIUM]

**Scenario:** Multiple overflow windows unfloat when room opens.

- Open 4 windows with maxWindows=2 (win-3, win-4 overflow)
- Close win-1 and win-2 (both tiled), set zones for 2
- Call retile with maxWindows=4
- Verify both win-3 and win-4 are auto-unfloated
- Verify both emit `windowFloatingChanged(wid, false, screen)`

### Issue 3.4 — `testOverflow_userFloatClearsTracking()` [MEDIUM]

**Scenario:** User explicitly floating an overflow window clears overflow status.

- Open win-1, win-2 with maxWindows=1 (win-2 overflows)
- Call `floatWindow("win-2")` — user explicitly floats it
- Increase maxWindows to 2
- Retile — win-2 should NOT auto-unfloat (user chose to float it)

### Issue 3.5 — `testOverflow_reentrantRetile()` [MEDIUM]

**Scenario:** Overflow recovery during an active retile (re-entrance path).

- Setup: 3 windows, maxWindows=2, win-3 overflows
- Close win-1 during an active retile cycle
- Verify the `m_retiling` branch still calls overflow recovery
- Verify win-3 is correctly unfloated

### Issue 3.6 — `testOverflow_multiScreenRemoval()` [MEDIUM]

**Scenario:** Removing one screen cleans overflow for only that screen.

- Setup: ScreenA with win-1 (tiled) + win-2 (overflow), ScreenB with win-3 (tiled) + win-4 (overflow)
- Remove ScreenA from autotile via `setAutotileScreens({ScreenB})`
- Verify: win-2 overflow tracking cleared (ScreenA gone)
- Verify: win-4 overflow tracking preserved (ScreenB still active)
- Verify: win-2 is NOT saved to `m_savedFloatingWindows` (overflow, not user-floated)

### Issue 3.7 — `testOverflow_perScreenMaxWindows()` [LOW]

**Scenario:** Per-screen maxWindows override affects overflow behavior.

- Set global maxWindows=3, per-screen override for ScreenA=1
- Open 2 windows on ScreenA
- Verify win-2 overflows on ScreenA (respects per-screen limit)
- Open 3 windows on ScreenB — all tile normally (global limit)

### Issue 3.8 — `testOverflow_toggleFloatClearsTracking()` [LOW]

**Scenario:** `performToggleFloat()` on an overflow window clears tracking.

- Open win-1, win-2 with maxWindows=1 (win-2 overflows)
- Call `toggleWindowFloat("win-2", screenName)` — toggles to tiled
- Verify overflow tracking cleared for win-2

---

## Phase 4: OverflowManager Unit Tests

### Issue 4.1 — Create `tests/unit/test_overflow_manager.cpp`

**New file.** Tests OverflowManager in isolation (no AutotileEngine dependency).

Test methods:
- `testMarkAndClear()` — basic add/remove
- `testIsOverflow()` — query correctness
- `testRecoverIfRoom_basicRecovery()` — single window unfloated
- `testRecoverIfRoom_noRoom()` — no room, nothing happens
- `testRecoverIfRoom_partialRoom()` — room for 1 of 3 overflow windows
- `testRemoveScreen()` — screen removal cleans all entries
- `testMigrateWindow()` — move between screens
- `testTakeOverflowForScreen()` — deactivation returns + removes
- `testPerScreenIsolation()` — operations on ScreenA don't affect ScreenB
- `testClearOverflowUnknownWindow()` — no-crash on unknown window

### Issue 4.2 — Add to CMakeLists.txt test target

Add `tests/unit/test_overflow_manager.cpp` to the test executable's source list.

---

## Execution Order & Dependencies

```
Phase 1 (no dependencies — new files only)
  1.1 → 1.2 → 1.3  (create class, implement, wire to build)

Phase 2 (depends on Phase 1 — modifies existing files)
  2.1 → 2.2, 2.3, 2.4, 2.5, 2.6  (header first, then all .cpp changes in parallel)

Phase 3 (depends on Phase 2 — tests validate the rewired engine)
  3.1 through 3.8  (all independent, can be written in any order)

Phase 4 (depends on Phase 1 only — tests the isolated class)
  4.1 → 4.2  (can run in parallel with Phase 2/3)
```

**Critical constraint:** All existing tests must pass after each phase. Phase 2 is the risky phase — do it as one atomic commit with zero behavioral changes.

---

## Estimated Impact

| Metric | Value |
|--------|-------|
| New files | 3 (`OverflowManager.h`, `OverflowManager.cpp`, `test_overflow_manager.cpp`) |
| Lines added | ~250 (class ~80, integration ~40, tests ~130) |
| Lines removed | ~120 (inline overflow logic in AutotileEngine) |
| Net change | ~+130 lines |
| Methods removed from AutotileEngine | 2 (`unfloatOverflowIfRoom`, `clearOverflowStatus`) |
| Members removed from AutotileEngine | 1 (`m_overflowWindows`) |
| New test methods | 18 (8 engine integration + 10 isolated OverflowManager) |
| Risk | LOW — pure structural extraction, zero behavioral changes |

---

## Validation Checklist

- [ ] All 33 existing `test_autotile_engine` tests pass
- [ ] All 76+ existing `test_tiling_algorithms` tests pass
- [ ] All 18 new tests pass
- [ ] `m_overflowWindows` grep returns zero hits in AutotileEngine
- [ ] `clearOverflowStatus` grep returns zero hits
- [ ] `unfloatOverflowIfRoom` grep returns zero hits
- [ ] Build succeeds with no warnings
- [ ] Signal ordering preserved (batch mutate → batch emit pattern maintained)
