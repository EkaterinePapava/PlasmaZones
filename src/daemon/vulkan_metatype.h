// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QVulkanInstance>
Q_DECLARE_METATYPE(QVulkanInstance*)

// Shared property name for passing the QVulkanInstance* between main.cpp and OverlayService.
// Using a constant avoids silent nullptr from typos on either side.
constexpr const char* PzVulkanInstanceProperty = "_pz_vulkanInstance";
