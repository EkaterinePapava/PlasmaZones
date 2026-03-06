# Unify Snapping & Autotile — Deferred Items

Items from the unification plan that were not implemented. Grouped by risk/effort.

---

## Completed (this session)

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
- [x] Kept `floatWindow`/`unfloatWindow` — needed for minimize/drag (explicit direction)

### PR 4: Feature Parity & Polish
- [x] 4A. OSD labels already correct for both modes — no change needed
- [x] 4B. Autotile `onWindowClosed` D-Bus call already gated internally
- [x] 4C. Fullscreen handler is entirely autotile-specific — no shared logic to extract

### Bug fixes found during review
- [x] Unconditional geometry stores corrupted float restore for autotile-only windows
- [x] `clearFloatingStateForSnap` passed stale screen to `windowFloatingChanged` signal
- [x] `setWindowFloating` used `m_lastActiveScreenName` — now looks up tracked screen
- [x] Eviction in pre-snap/pre-autotile caches could self-evict just-inserted entries

---

## Deferred: PR 3 — Remove `floatWindow`/`unfloatWindow` from Autotile D-Bus

The plan called for removing all autotile float D-Bus methods. We kept `floatWindow` and
`unfloatWindow` because the minimize handler and drag-to-float handler need explicit
directional float/unfloat (not a toggle). Removing them requires either:

1. Adding a unified `setWindowFloating(windowId, screenName, floating)` to WTA that routes
   to the autotile engine's `floatWindow`/`unfloatWindow` for autotile screens, or
2. Using `toggleFloatForWindow` with state guards — risky due to race conditions between
   the effect's local floating cache and the daemon's state.

**Effort:** Medium. **Risk:** Medium (minimize/unminimize edge cases).

**Files involved:**
- `dbus/org.plasmazones.Autotile.xml` — remove `floatWindow`, `unfloatWindow`
- `src/dbus/autotileadaptor.h` + `.cpp` — remove implementations
- `kwin-effect/autotilehandler/signals.cpp:337` — update minimize handler
- `kwin-effect/plasmazoneseffect.cpp:294` — update drag-to-float handler

---

## Deferred: Tier 3 — Architectural Refactors

### Merge pre-snap + pre-autotile into single geometry storage

Both modes store "geometry before tiling" with different keys and different semantics
(first-only vs always-overwrite). Merging would reduce complexity but touches core
persistence (save/load, session restore) and requires careful handling of the semantic
differences.

**Effort:** High. **Risk:** High — breaks persistence format, affects session restore.

**Files:**
- `src/core/windowtrackingservice.h` — merge `m_preSnapGeometries` + `m_preAutotileGeometries`
- `src/core/windowtrackingservice.cpp` — unify store/retrieve/clear logic
- `src/dbus/windowtrackingadaptor/persistence.cpp` — update save/load
- `src/dbus/windowtrackingadaptor/float.cpp` — update `applyGeometryForFloat` fallback chain
- `kwin-effect/autotilehandler.cpp` — update `saveAndRecordPreAutotileGeometry`

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

### Unify `windowOpened` D-Bus signature

WTS `windowOpened` takes `(windowId)` via `notifyWindowOpened`. Autotile takes
`(windowId, screenName, minWidth, minHeight)`. Extending WTS to accept the extra params
would allow the daemon to route window-open notifications based on mode.

**Effort:** Low. **Risk:** Low.

### Rename `getValidatedPreSnapGeometry` → `getValidatedPreTileGeometry`

Cosmetic rename to reflect that it now serves both snapping and autotile modes via
the `applyGeometryForFloat` fallback chain.

**Effort:** Trivial. **Risk:** None (internal API only).
