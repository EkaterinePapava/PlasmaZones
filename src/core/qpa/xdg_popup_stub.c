// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Stub for xdg_popup_interface referenced by the wlr-layer-shell protocol
// code (for the get_popup request). PlasmaZones never uses layer shell
// popups, but the generated protocol code references this symbol.

#include <wayland-util.h>

// Minimal stub — only needs to exist for the linker. The generated
// wlr-layer-shell protocol code references this symbol for the
// get_popup request, but PlasmaZones never calls get_popup. If this
// stub is ever reached at runtime, the NULL method arrays will cause
// an immediate crash — which is the correct behavior since it means
// we have a logic error (we should never create layer-shell popups).
const struct wl_interface xdg_popup_interface = {
    "xdg_popup", 1, 0, NULL, 0, NULL,
};
