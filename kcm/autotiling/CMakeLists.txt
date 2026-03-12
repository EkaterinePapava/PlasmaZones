# PlasmaZones Autotiling sub-KCM
# SPDX-License-Identifier: GPL-3.0-or-later

set(kcm_autotiling_SRCS
    kcmautotiling.cpp
    kcmautotiling.h
)

# KConfigXT — own copy of generated symbols (needed by ConfigDefaults, which
# is header-only and instantiates PlasmaZonesConfig inline)
kconfig_add_kcfg_files(kcm_autotiling_kcfg_SRCS
    ../../src/config/plasmazones.kcfgc
    GENERATE_MOC
)

kcmutils_add_qml_kcm(kcm_plasmazones_autotiling
    SOURCES ${kcm_autotiling_SRCS} ${kcm_autotiling_kcfg_SRCS}
)

# Embed shared QML components used by this KCM
set(_kcm_qml_prefix "/kcm/kcm_plasmazones_autotiling")

qt_add_resources(kcm_plasmazones_autotiling "kcm_autotiling_shared_qml"
    PREFIX "${_kcm_qml_prefix}"
    BASE "../ui"
    FILES
        ../ui/ColorButton.qml
        ../ui/PerScreenOverrideHelper.qml
        ../ui/MonitorSelectorSection.qml
        ../ui/ScreenComboBox.qml
        ../ui/WideComboBox.qml
)

# Embed AlgorithmPreview from tabs
qt_add_resources(kcm_plasmazones_autotiling "kcm_autotiling_tabs_qml"
    PREFIX "${_kcm_qml_prefix}"
    BASE "../ui/tabs"
    FILES
        ../ui/tabs/AlgorithmPreview.qml
)

# Embed shared QML types (ZonePreview) so that
# `import org.plasmazones.common` resolves from qrc without needing
# the static plugin (which cannot be linked into sub-KCMs).
qt_add_resources(kcm_plasmazones_autotiling "kcm_autotiling_shared_types"
    PREFIX "/qt/qml/org/plasmazones/common"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/../../src/shared"
    FILES
        ../../src/shared/ZonePreview.qml
)
qt_add_resources(kcm_plasmazones_autotiling "kcm_autotiling_shared_qmldir"
    PREFIX "/qt/qml/org/plasmazones/common"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/../common/org_plasmazones_common"
    FILES
        ../common/org_plasmazones_common/qmldir
)

target_link_libraries(kcm_plasmazones_autotiling
    PRIVATE
        plasmazones_core
        Qt6::DBus
        KF6::KCMUtils
        KF6::ConfigWidgets
)
