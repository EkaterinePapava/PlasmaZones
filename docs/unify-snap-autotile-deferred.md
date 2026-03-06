# Unify Snapping & Autotile — Status

Tracks what was completed and what remains from the unification plan.

---

## Completed

### PR 1: Signal & Label Unification
- [x] 1A. Unified `windowFloatingChanged` to 3-param (windowId, isFloating, screenName)
- [x] 1B. Resnap/retile OSD suppression — expanded check to cover `"retile"` action
- [x] 1C. Pre-autotile geometry cache 100-entry eviction cap
- [x] 1D. Skipped — autotile-only signal handlers already early-return for non-autotile screens

### PR 2: Unified Float Toggle (daemon-routed)
- [x] WTA routes `toggleFloatForWindow` to autotile engine via `std::function` callbacks
- [x] Effect stores mode-appropriate geometry before calling unified method
- [x] Daemon clears callbacks on shutdown (prevents use-after-free)

### PR 3: Deprecate Redundant Autotile D-Bus Float Methods
- [x] Removed `toggleFocusedWindowFloat` and `toggleWindowFloat` from Autotile D-Bus
- [x] Removed dead `handleAutotileFloatToggle` from effect
- [x] Added `setWindowFloatingForScreen(windowId, screenName, floating)` to WTA D-Bus
- [x] WTA routes to autotile engine via `m_autotileSetFloat` callback for autotile screens
- [x] Updated minimize handler, drag-to-float, and monocle unmaximize to use unified method
- [x] Removed `floatWindow`/`unfloatWindow` from Autotile D-Bus XML and adaptor

### PR 4: Feature Parity & Polish
- [x] 4A. OSD labels already correct for both modes — no change needed
- [x] 4B. Autotile `onWindowClosed` D-Bus call already gated internally
- [x] 4C. Fullscreen handler is entirely autotile-specific — no shared logic to extract

### Rename: `getValidatedPreSnapGeometry` -> `getValidatedPreTileGeometry`
- [x] Renamed across D-Bus XML, WTA header, persistence.cpp, targets.cpp,
      navigationhandler.cpp, windowdragadaptor drop.cpp and drag.cpp

### Tier 3 #1: Merge pre-snap + pre-autotile into single geometry storage
- [x] Replaced `m_preSnapGeometries` + `m_preAutotileGeometries` with unified `m_preTileGeometries`
- [x] `storePreTileGeometry(windowId, geometry, overwrite)` — `false` for snap (first-only), `true` for autotile (always-overwrite)
- [x] Dual-key storage (windowId + appId) now applied to both modes, improving snap-mode session restore
- [x] Session persistence migration: reads old `PreSnapGeometries`/`PreAutotileGeometries`, merges into `PreTileGeometries` (pre-snap wins on conflict)
- [x] `windowClosed` cleanup simplified: removes stale windowId key, appId key survives for session restore
- [x] Unified D-Bus API: `storePreTileGeometry`, `hasPreTileGeometry`, `clearPreTileGeometry`, `getValidatedPreTileGeometry`
- [x] Removed `floatWindow`/`unfloatWindow` from Autotile D-Bus XML (callers migrated to `setWindowFloatingForScreen`)
- [x] Effect callers updated: `ensurePreSnapGeometryStored`, `slotToggleWindowFloatRequested`, drag-to-float, navigation handler, autotile handler
- [x] All 46 tests updated and passing

### Bug fixes found during analysis
- [x] Eviction convergence bug: dual-key insertion added 2 entries but evicted only 1, causing slow cache growth — fixed with `while` loop
- [x] DRY: extracted `unfloatToZone()` private helper — deduplicated ~30 lines shared between `toggleFloatForWindow` and `setWindowFloatingForScreen`
- [x] Drag-to-float screen lookup race: `getWindowScreenName` after drag returns destination screen, not origin — fixed by capturing `m_dragBypassScreenName` at drag start

### Earlier bug fixes (PR 1-3 review)
- [x] Unconditional geometry stores corrupted float restore for autotile-only windows
- [x] `clearFloatingStateForSnap` passed stale screen to `windowFloatingChanged` signal
- [x] `setWindowFloating` used `m_lastActiveScreenName` — now looks up tracked screen
- [x] Eviction in pre-snap/pre-autotile caches could self-evict just-inserted entries

---

## Deferred: Unify `windowOpened` D-Bus signature

WTS has no `windowOpened` D-Bus method. Autotile has
`windowOpened(windowId, screenName, minWidth, minHeight)`. The idea was to add a unified
method to WTA that routes to the autotile engine for autotile screens.

**Why deferred:** The effect's `AutotileHandler::notifyWindowAdded` maintains critical
local state (`m_notifiedWindows` deduplication, `m_pendingCloses` race-condition guard)
that would need to move to the daemon or be replicated. The current routing is clean —
the effect already gates by screen type locally. Moving this to the daemon provides
minimal benefit for non-trivial rework.

**Effort:** Medium (higher than originally estimated). **Risk:** Medium.

**Files that would be involved:**
- `dbus/org.plasmazones.WindowTracking.xml` — add `windowOpened` method
- `src/dbus/windowtrackingadaptor.h` + `/float.cpp` — implement with autotile routing callback
- `kwin-effect/autotilehandler.cpp` — change D-Bus target from Autotile to WindowTracking
- `src/daemon/daemon.cpp` — wire new callback

---

## Deferred: Tier 3 — Architectural Refactors

### ~~Merge pre-snap + pre-autotile into single geometry storage~~ ✓ COMPLETED

Completed above — see "Tier 3 #1" in Completed section.

### Split `applyGeometryRequested` signal by purpose

Currently one signal serves float restore (empty zoneId) and snap restore (zoneId set).
Splitting would make the effect's handler clearer but requires a D-Bus schema change
coordinated between daemon and effect.

**Effort:** Medium. **Risk:** Medium — D-Bus interface change.

### Add minimize/maximize tracking for snapping mode

Autotile tracks minimize (float on minimize, unfloat on unminimize). Snapping mode doesn't.
Adding this would give feature parity but needs new D-Bus methods on WTS and new signal
connections in the effect.

**Effort:** Medium. **Risk:** Low-Medium.
