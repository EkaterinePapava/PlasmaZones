// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtWaylandClient/private/qwaylandshellintegration_p.h>
#include "../plasmazones_export.h"
#include "wlr_layer_shell_protocol.h"

namespace PlasmaZones {

/// QPA shell integration plugin that binds zwlr_layer_shell_v1 and creates
/// layer surfaces for windows marked with the _pz_layer_shell property.
class PLASMAZONES_EXPORT LayerShellIntegration : public QtWaylandClient::QWaylandShellIntegration
{
public:
    LayerShellIntegration();
    ~LayerShellIntegration() override;

    bool initialize(QtWaylandClient::QWaylandDisplay* display) override;
    QtWaylandClient::QWaylandShellSurface* createShellSurface(QtWaylandClient::QWaylandWindow* window) override;

    struct zwlr_layer_shell_v1* layerShell() const
    {
        return m_globalAvailable ? m_layerShell : nullptr;
    }

    /// Access the singleton instance (available after Qt loads the plugin).
    static LayerShellIntegration* instance();

    // Public for C callback struct initialization
    static void registryHandler(void* data, struct wl_registry* registry, uint32_t id, const char* interface,
                                uint32_t version);
    static void registryRemoveHandler(void* data, struct wl_registry* registry, uint32_t id);

private:
    struct zwlr_layer_shell_v1* m_layerShell = nullptr;
    struct wl_registry* m_registry = nullptr;
    uint32_t m_layerShellId = 0;
    uint32_t m_boundVersion = 0;
    // Tracks whether the compositor's global is still advertised.
    // When false, layerShell() returns nullptr to prevent new surface creation,
    // but m_layerShell is kept non-null for proper cleanup in the destructor
    // (avoids leaking the wl_proxy when the global is removed at runtime).
    bool m_globalAvailable = false;

    static LayerShellIntegration* s_instance;
};

} // namespace PlasmaZones
