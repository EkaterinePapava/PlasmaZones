// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

/**
 * @brief Filter bar for layout/algorithm grid — group by, sort by, filters, and text search.
 *
 * Options change dynamically based on viewMode (0 = Snapping, 1 = Tiling).
 */
RowLayout {
    id: root

    property int viewMode: 0
    // ── Exposed state: group / sort ─────────────────────────────────────────
    property int groupByIndex: 0
    property int sortByIndex: 0
    property bool sortAscending: true
    property string filterText: ""
    // ── Exposed state: shared filters ───────────────────────────────────────
    property bool showHidden: false
    // ── Exposed state: snapping filters (all ON = show everything) ──────────
    property bool showAspectAny: true
    property bool showAspectStandard: true
    property bool showAspectUltrawide: true
    property bool showAspectSuperUltrawide: true
    property bool showAspectPortrait: true
    property bool showAutoAssignOnly: false
    property bool showBuiltInLayouts: true
    property bool showUserLayouts: true
    // ── Exposed state: tiling filters ───────────────────────────────────────
    property bool showSystemAlgorithms: true
    property bool showUserAlgorithms: true
    property bool onlyMasterCount: false
    property bool onlySplitRatio: false
    property bool onlyOverlapping: false
    property bool onlyPersistent: false
    // Whether any non-default filter is active (drives badge visibility)
    readonly property bool hasActiveFilters: {
        if (filterText.length > 0)
            return true;

        if (root.viewMode === 0)
            return !showAspectAny || !showAspectStandard || !showAspectUltrawide || !showAspectSuperUltrawide || !showAspectPortrait || showHidden || showAutoAssignOnly || !showBuiltInLayouts || !showUserLayouts;
        else
            return !showSystemAlgorithms || !showUserAlgorithms || showHidden || onlyMasterCount || onlySplitRatio || onlyOverlapping || onlyPersistent;
    }
    // Static ComboBox models (avoids inline array recreation that resets currentIndex)
    readonly property var snappingGroupModel: [i18n("Aspect Ratio"), i18n("Zone Count"), i18n("Auto / Manual"), i18n("Source"), i18n("None")]
    readonly property var tilingGroupModel: [i18n("Capability"), i18n("Source"), i18n("Persistent"), i18n("None")]
    readonly property var sortModel: [i18n("Name"), i18n("Zone Count")]

    signal filterSettingsChanged()

    function resetFilters() {
        filterText = "";
        searchField.clear();
        showHidden = false;
        showAspectAny = true;
        showAspectStandard = true;
        showAspectUltrawide = true;
        showAspectSuperUltrawide = true;
        showAspectPortrait = true;
        showAutoAssignOnly = false;
        showBuiltInLayouts = true;
        showUserLayouts = true;
        showSystemAlgorithms = true;
        showUserAlgorithms = true;
        onlyMasterCount = false;
        onlySplitRatio = false;
        onlyOverlapping = false;
        onlyPersistent = false;
    }

    spacing: Kirigami.Units.smallSpacing
    // Reset all state when view mode changes
    onViewModeChanged: {
        root.groupByIndex = 0;
        root.sortByIndex = 0;
        root.sortAscending = true;
        resetFilters();
        groupByCombo.currentIndex = 0;
        sortByCombo.currentIndex = 0;
        root.filterSettingsChanged();
    }

    // ── Group By ────────────────────────────────────────────────────────────
    Label {
        text: i18n("Group:")
        font: Kirigami.Theme.smallFont
        color: Kirigami.Theme.disabledTextColor
    }

    ComboBox {
        id: groupByCombo

        Layout.preferredWidth: Kirigami.Units.gridUnit * 8
        model: root.viewMode === 0 ? root.snappingGroupModel : root.tilingGroupModel
        currentIndex: root.groupByIndex
        onActivated: (index) => {
            root.groupByIndex = index;
            root.filterSettingsChanged();
        }
    }

    // ── Sort By ─────────────────────────────────────────────────────────────
    Label {
        text: i18n("Sort:")
        font: Kirigami.Theme.smallFont
        color: Kirigami.Theme.disabledTextColor
    }

    ComboBox {
        id: sortByCombo

        Layout.preferredWidth: Kirigami.Units.gridUnit * 8
        model: root.sortModel
        currentIndex: root.sortByIndex
        onActivated: (index) => {
            root.sortByIndex = index;
            root.filterSettingsChanged();
        }
    }

    ToolButton {
        icon.name: root.sortAscending ? "view-sort-ascending" : "view-sort-descending"
        icon.width: Kirigami.Units.iconSizes.smallMedium
        icon.height: Kirigami.Units.iconSizes.smallMedium
        onClicked: {
            root.sortAscending = !root.sortAscending;
            root.filterSettingsChanged();
        }
        Accessible.name: i18n("Toggle sort direction")
        ToolTip.visible: hovered
        ToolTip.text: root.sortAscending ? i18n("Ascending") : i18n("Descending")
    }

    Item {
        Layout.fillWidth: true
    }

    // ── Search ──────────────────────────────────────────────────────────────
    TextField {
        id: searchField

        Layout.preferredWidth: Kirigami.Units.gridUnit * 12
        placeholderText: root.viewMode === 0 ? i18n("Search layouts\u2026") : i18n("Search algorithms\u2026")
        inputMethodHints: Qt.ImhNoPredictiveText
        rightPadding: clearButton.visible ? clearButton.width + Kirigami.Units.smallSpacing : undefined
        onTextChanged: {
            root.filterText = text;
            root.filterSettingsChanged();
        }

        ToolButton {
            id: clearButton

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            visible: searchField.text.length > 0
            icon.name: "edit-clear"
            icon.width: Kirigami.Units.iconSizes.small
            icon.height: Kirigami.Units.iconSizes.small
            onClicked: searchField.clear()
            Accessible.name: i18n("Clear search")
        }

    }

    // ── Filter Button ───────────────────────────────────────────────────────
    ToolButton {
        id: filterButton

        icon.name: "view-filter"
        checked: root.hasActiveFilters
        onClicked: {
            if (root.viewMode === 0)
                snappingFilterMenu.popup();
            else
                tilingFilterMenu.popup();
        }
        Accessible.name: root.hasActiveFilters ? i18n("Filter (active)") : i18n("Filter")
        ToolTip.visible: hovered
        ToolTip.text: root.hasActiveFilters ? i18n("Filters active \u2014 click to change") : i18n("Filter")
    }

    // ── Snapping Filter Menu ────────────────────────────────────────────────
    Menu {
        id: snappingFilterMenu

        title: i18n("Filter Layouts")

        MenuItem {
            text: i18n("Built-in")
            checkable: true
            checked: root.showBuiltInLayouts
            onToggled: {
                root.showBuiltInLayouts = checked;
                root.filterSettingsChanged();
            }
        }

        MenuItem {
            text: i18n("User Layouts")
            checkable: true
            checked: root.showUserLayouts
            onToggled: {
                root.showUserLayouts = checked;
                root.filterSettingsChanged();
            }
        }

        MenuSeparator {
        }

        MenuItem {
            text: i18n("All Monitors")
            checkable: true
            checked: root.showAspectAny
            onToggled: {
                root.showAspectAny = checked;
                root.filterSettingsChanged();
            }
        }

        MenuItem {
            text: i18n("Standard (16:9)")
            checkable: true
            checked: root.showAspectStandard
            onToggled: {
                root.showAspectStandard = checked;
                root.filterSettingsChanged();
            }
        }

        MenuItem {
            text: i18n("Ultrawide (21:9)")
            checkable: true
            checked: root.showAspectUltrawide
            onToggled: {
                root.showAspectUltrawide = checked;
                root.filterSettingsChanged();
            }
        }

        MenuItem {
            text: i18n("Super-Ultrawide (32:9)")
            checkable: true
            checked: root.showAspectSuperUltrawide
            onToggled: {
                root.showAspectSuperUltrawide = checked;
                root.filterSettingsChanged();
            }
        }

        MenuItem {
            text: i18n("Portrait (9:16)")
            checkable: true
            checked: root.showAspectPortrait
            onToggled: {
                root.showAspectPortrait = checked;
                root.filterSettingsChanged();
            }
        }

        MenuSeparator {
        }

        MenuItem {
            text: i18n("Auto Only")
            checkable: true
            checked: root.showAutoAssignOnly
            onToggled: {
                root.showAutoAssignOnly = checked;
                root.filterSettingsChanged();
            }
        }

        MenuSeparator {
        }

        MenuItem {
            text: i18n("Show Hidden Layouts")
            checkable: true
            checked: root.showHidden
            onToggled: {
                root.showHidden = checked;
                root.filterSettingsChanged();
            }
        }

        MenuSeparator {
        }

        MenuItem {
            text: i18n("Reset Filters")
            icon.name: "edit-reset"
            enabled: root.hasActiveFilters
            onTriggered: {
                root.resetFilters();
                root.filterSettingsChanged();
            }
        }

    }

    // ── Tiling Filter Menu ──────────────────────────────────────────────────
    Menu {
        id: tilingFilterMenu

        title: i18n("Filter Algorithms")

        MenuItem {
            text: i18n("Built-in")
            checkable: true
            checked: root.showSystemAlgorithms
            onToggled: {
                root.showSystemAlgorithms = checked;
                root.filterSettingsChanged();
            }
        }

        MenuItem {
            text: i18n("User Scripts")
            checkable: true
            checked: root.showUserAlgorithms
            onToggled: {
                root.showUserAlgorithms = checked;
                root.filterSettingsChanged();
            }
        }

        MenuSeparator {
        }

        MenuItem {
            text: i18n("Capabilities (any match):")
            enabled: false
        }

        MenuItem {
            text: i18n("Master Count")
            checkable: true
            checked: root.onlyMasterCount
            onToggled: {
                root.onlyMasterCount = checked;
                root.filterSettingsChanged();
            }
        }

        MenuItem {
            text: i18n("Split Ratio")
            checkable: true
            checked: root.onlySplitRatio
            onToggled: {
                root.onlySplitRatio = checked;
                root.filterSettingsChanged();
            }
        }

        MenuItem {
            text: i18n("Overlapping Zones")
            checkable: true
            checked: root.onlyOverlapping
            onToggled: {
                root.onlyOverlapping = checked;
                root.filterSettingsChanged();
            }
        }

        MenuItem {
            text: i18n("Persistent (Memory)")
            checkable: true
            checked: root.onlyPersistent
            onToggled: {
                root.onlyPersistent = checked;
                root.filterSettingsChanged();
            }
        }

        MenuSeparator {
        }

        MenuItem {
            text: i18n("Show Hidden Algorithms")
            checkable: true
            checked: root.showHidden
            onToggled: {
                root.showHidden = checked;
                root.filterSettingsChanged();
            }
        }

        MenuSeparator {
        }

        MenuItem {
            text: i18n("Reset Filters")
            icon.name: "edit-reset"
            enabled: root.hasActiveFilters
            onTriggered: {
                root.resetFilters();
                root.filterSettingsChanged();
            }
        }

    }

}
