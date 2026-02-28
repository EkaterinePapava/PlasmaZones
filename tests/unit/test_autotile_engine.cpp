// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QCoreApplication>
#include <QSignalSpy>

#include "autotile/AutotileEngine.h"
#include "autotile/AutotileConfig.h"
#include "autotile/TilingState.h"
#include "autotile/AlgorithmRegistry.h"
#include "core/constants.h"

using namespace PlasmaZones;

/**
 * @brief Unit tests for AutotileEngine
 *
 * Tests cover:
 * - Enable/disable functionality
 * - Algorithm selection and switching
 * - Tiling state management
 * - Manual tiling operations
 * - Master count and split ratio adjustments
 */
class TestAutotileEngine : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // ═══════════════════════════════════════════════════════════════════════════
    // Setup/Teardown
    // ═══════════════════════════════════════════════════════════════════════════

    void initTestCase()
    {
        // Ensure AlgorithmRegistry is initialized
        AlgorithmRegistry::instance();
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Constructor tests
    // ═══════════════════════════════════════════════════════════════════════════

    void testConstruction_defaultValues()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);

        QVERIFY(!engine.isEnabled());
        QCOMPARE(engine.algorithm(), AlgorithmRegistry::defaultAlgorithmId());
        QVERIFY(engine.config() != nullptr);
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Enable/disable tests
    // ═══════════════════════════════════════════════════════════════════════════

    void testEnabled_initiallyFalse()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        QVERIFY(!engine.isEnabled());
    }

    void testEnabled_setTrue()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        QSignalSpy spy(&engine, &AutotileEngine::enabledChanged);

        QSet<QString> screens{QStringLiteral("HDMI-1")};
        engine.setAutotileScreens(screens);

        QVERIFY(engine.isEnabled());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toBool(), true);
    }

    void testEnabled_noChangeNoSignal()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        QSignalSpy spy(&engine, &AutotileEngine::enabledChanged);

        engine.setAutotileScreens({}); // Already empty

        QVERIFY(!engine.isEnabled());
        QCOMPARE(spy.count(), 0);
    }

    void testEnabled_toggleBackAndForth()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        QSignalSpy spy(&engine, &AutotileEngine::enabledChanged);

        QSet<QString> screens{QStringLiteral("HDMI-1")};
        engine.setAutotileScreens(screens);  // off → on
        engine.setAutotileScreens({});        // on → off
        engine.setAutotileScreens(screens);  // off → on

        QVERIFY(engine.isEnabled());
        QCOMPARE(spy.count(), 3);
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Algorithm selection tests
    // ═══════════════════════════════════════════════════════════════════════════

    void testAlgorithm_defaultIsMasterStack()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        QCOMPARE(engine.algorithm(), DBus::AutotileAlgorithm::MasterStack);
    }

    void testAlgorithm_setValid()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        QSignalSpy spy(&engine, &AutotileEngine::algorithmChanged);

        engine.setAlgorithm(DBus::AutotileAlgorithm::Columns);

        QCOMPARE(engine.algorithm(), DBus::AutotileAlgorithm::Columns);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toString(), DBus::AutotileAlgorithm::Columns);
    }

    void testAlgorithm_setInvalidFallsBackToDefault()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);

        // First set to something valid that's not default
        engine.setAlgorithm(DBus::AutotileAlgorithm::BSP);
        QCOMPARE(engine.algorithm(), DBus::AutotileAlgorithm::BSP);

        // Now set to invalid - should fall back to default
        QSignalSpy spy(&engine, &AutotileEngine::algorithmChanged);
        engine.setAlgorithm(QStringLiteral("nonexistent-algorithm"));

        QCOMPARE(engine.algorithm(), AlgorithmRegistry::defaultAlgorithmId());
        QCOMPARE(spy.count(), 1);
    }

    void testAlgorithm_sameValueNoSignal()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        QSignalSpy spy(&engine, &AutotileEngine::algorithmChanged);

        engine.setAlgorithm(engine.algorithm()); // Same value

        QCOMPARE(spy.count(), 0);
    }

    void testAlgorithm_currentAlgorithmNotNull()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        QVERIFY(engine.currentAlgorithm() != nullptr);
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // State management tests
    // ═══════════════════════════════════════════════════════════════════════════

    void testStateForScreen_createNew()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);

        TilingState *state = engine.stateForScreen(QStringLiteral("TestScreen"));

        QVERIFY(state != nullptr);
        QCOMPARE(state->screenName(), QStringLiteral("TestScreen"));
    }

    void testStateForScreen_returnsSameInstance()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);

        TilingState *state1 = engine.stateForScreen(QStringLiteral("TestScreen"));
        TilingState *state2 = engine.stateForScreen(QStringLiteral("TestScreen"));

        QCOMPARE(state1, state2);
    }

    void testStateForScreen_differentScreensDifferentStates()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);

        TilingState *state1 = engine.stateForScreen(QStringLiteral("Screen1"));
        TilingState *state2 = engine.stateForScreen(QStringLiteral("Screen2"));

        QVERIFY(state1 != state2);
        QCOMPARE(state1->screenName(), QStringLiteral("Screen1"));
        QCOMPARE(state2->screenName(), QStringLiteral("Screen2"));
    }

    void testStateForScreen_inheritsConfigDefaults()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        AutotileConfig *config = engine.config();
        config->masterCount = 2;
        config->splitRatio = 0.7;

        TilingState *state = engine.stateForScreen(QStringLiteral("TestScreen"));

        // Note: masterCount gets clamped by TilingState based on actual window count
        // When no windows, it clamps to MinMasterCount (1)
        // Split ratio should be set correctly
        QCOMPARE(state->splitRatio(), 0.7);

        // Add windows and verify masterCount can expand
        state->addWindow(QStringLiteral("win1"));
        state->addWindow(QStringLiteral("win2"));
        state->setMasterCount(2);
        QCOMPARE(state->masterCount(), 2);
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config access tests
    // ═══════════════════════════════════════════════════════════════════════════

    void testConfig_notNull()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        QVERIFY(engine.config() != nullptr);
    }

    void testConfig_modifiable()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        AutotileConfig *config = engine.config();

        config->innerGap = 20;
        config->outerGap = 15;

        QCOMPARE(engine.config()->innerGap, 20);
        QCOMPARE(engine.config()->outerGap, 15);
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Master ratio adjustment tests
    // ═══════════════════════════════════════════════════════════════════════════

    void testIncreaseMasterRatio_updatesAllScreens()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);

        // Create states for two screens
        TilingState *state1 = engine.stateForScreen(QStringLiteral("Screen1"));
        TilingState *state2 = engine.stateForScreen(QStringLiteral("Screen2"));

        const qreal initial1 = state1->splitRatio();
        const qreal initial2 = state2->splitRatio();

        engine.increaseMasterRatio(0.1);

        QVERIFY(qFuzzyCompare(state1->splitRatio(), initial1 + 0.1));
        QVERIFY(qFuzzyCompare(state2->splitRatio(), initial2 + 0.1));
    }

    void testDecreaseMasterRatio_updatesAllScreens()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);

        TilingState *state = engine.stateForScreen(QStringLiteral("Screen1"));
        const qreal initial = state->splitRatio();

        engine.decreaseMasterRatio(0.1);

        QVERIFY(qFuzzyCompare(state->splitRatio(), initial - 0.1));
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Master count adjustment tests
    // ═══════════════════════════════════════════════════════════════════════════

    void testIncreaseMasterCount_updatesAllScreens()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);

        TilingState *state = engine.stateForScreen(QStringLiteral("Screen1"));

        // Add some windows so master count can increase
        state->addWindow(QStringLiteral("win1"));
        state->addWindow(QStringLiteral("win2"));
        state->addWindow(QStringLiteral("win3"));

        const int initial = state->masterCount();

        engine.increaseMasterCount();

        QCOMPARE(state->masterCount(), initial + 1);
    }

    void testDecreaseMasterCount_updatesAllScreens()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);

        TilingState *state = engine.stateForScreen(QStringLiteral("Screen1"));
        state->addWindow(QStringLiteral("win1"));
        state->addWindow(QStringLiteral("win2"));
        state->setMasterCount(2);

        engine.decreaseMasterCount();

        QCOMPARE(state->masterCount(), 1);
    }

    void testDecreaseMasterCount_doesNotGoBelowOne()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);

        TilingState *state = engine.stateForScreen(QStringLiteral("Screen1"));
        state->addWindow(QStringLiteral("win1"));
        QCOMPARE(state->masterCount(), 1);

        engine.decreaseMasterCount();

        QCOMPARE(state->masterCount(), 1); // Should stay at 1
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Retile tests (disabled engine)
    // ═══════════════════════════════════════════════════════════════════════════

    void testRetile_disabledEngineNoOp()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        QVERIFY(!engine.isEnabled());

        // This should not crash and should be a no-op
        engine.retile();
        engine.retile(QStringLiteral("SomeScreen"));
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Window lifecycle tests
    // ═══════════════════════════════════════════════════════════════════════════

    void testWindowLifecycle()
    {
        // Verify windowOpened/windowClosed update TilingState correctly
        // and tilingChanged signals are emitted.
        // Note: windowTiled signal requires a real ScreenManager to provide
        // screen geometry, so it is not emitted with null dependencies.
        AutotileEngine engine(nullptr, nullptr, nullptr);

        const QString screenName = QStringLiteral("TestScreen");
        const QString windowId = QStringLiteral("win-lifecycle-1");

        // Enable autotile on a screen
        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);
        QVERIFY(engine.isEnabled());

        // Spy on tilingChanged signal
        QSignalSpy tilingSpy(&engine, &AutotileEngine::tilingChanged);

        // Open a window
        engine.windowOpened(windowId, screenName);

        // Process the QueuedConnection retile
        QCoreApplication::processEvents();

        // Verify the window appears in the engine's tiling state
        TilingState *state = engine.stateForScreen(screenName);
        QVERIFY(state != nullptr);
        QVERIFY(state->containsWindow(windowId));
        QCOMPARE(state->windowCount(), 1);

        // tilingChanged should have been emitted for the screen
        QVERIFY(tilingSpy.count() >= 1);
        QCOMPARE(tilingSpy.last().first().toString(), screenName);

        // Close the window
        tilingSpy.clear();
        engine.windowClosed(windowId);

        // Process the QueuedConnection retile
        QCoreApplication::processEvents();

        // Verify cleanup
        QVERIFY(!state->containsWindow(windowId));
        QCOMPARE(state->windowCount(), 0);

        // tilingChanged should have been emitted for the close as well
        QVERIFY(tilingSpy.count() >= 1);
        QCOMPARE(tilingSpy.last().first().toString(), screenName);
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Config round-trip tests
    // ═══════════════════════════════════════════════════════════════════════════

    void testConfigRoundTrip()
    {
        AutotileConfig original;
        original.innerGap = 5;
        original.outerGap = 10;
        original.splitRatio = 0.65;
        original.masterCount = 2;
        original.algorithmId = QStringLiteral("bsp");
        original.smartGaps = false;
        original.focusNewWindows = false;
        original.focusFollowsMouse = true;
        original.monocleHideOthers = false;
        original.monocleShowTabs = true;
        original.respectMinimumSize = false;
        original.insertPosition = AutotileConfig::InsertPosition::AfterFocused;

        QJsonObject json = original.toJson();
        AutotileConfig restored = AutotileConfig::fromJson(json);

        QCOMPARE(restored.innerGap, original.innerGap);
        QCOMPARE(restored.outerGap, original.outerGap);
        QCOMPARE(restored.splitRatio, original.splitRatio);
        QCOMPARE(restored.masterCount, original.masterCount);
        QCOMPARE(restored.algorithmId, original.algorithmId);
        QCOMPARE(restored.smartGaps, original.smartGaps);
        QCOMPARE(restored.focusNewWindows, original.focusNewWindows);
        QCOMPARE(restored.focusFollowsMouse, original.focusFollowsMouse);
        QCOMPARE(restored.monocleHideOthers, original.monocleHideOthers);
        QCOMPARE(restored.monocleShowTabs, original.monocleShowTabs);
        QCOMPARE(restored.respectMinimumSize, original.respectMinimumSize);
        QCOMPARE(restored.insertPosition, original.insertPosition);
    }
    // ═══════════════════════════════════════════════════════════════════════════
    // windowMinSizeUpdated tests
    // ═══════════════════════════════════════════════════════════════════════════

    void testWindowMinSizeUpdated_validWindow()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        const QString windowId = QStringLiteral("win-minsize-1");

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);
        engine.windowOpened(windowId, screenName, 100, 50);
        QCoreApplication::processEvents();

        // Update min size — should trigger retile
        QSignalSpy tilingSpy(&engine, &AutotileEngine::tilingChanged);
        engine.windowMinSizeUpdated(windowId, 200, 100);
        QCoreApplication::processEvents();

        QVERIFY(tilingSpy.count() >= 1);
    }

    void testWindowMinSizeUpdated_noOpSameValue()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        const QString windowId = QStringLiteral("win-minsize-2");

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);
        engine.windowOpened(windowId, screenName, 100, 50);
        QCoreApplication::processEvents();

        QSignalSpy tilingSpy(&engine, &AutotileEngine::tilingChanged);
        // Same min size as initial — should be a no-op
        engine.windowMinSizeUpdated(windowId, 100, 50);
        QCoreApplication::processEvents();

        QCOMPARE(tilingSpy.count(), 0);
    }

    void testWindowMinSizeUpdated_unknownWindow()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);

        // Should not crash — window was never opened
        engine.windowMinSizeUpdated(QStringLiteral("nonexistent-win"), 100, 50);
        QCoreApplication::processEvents();
    }

    void testWindowMinSizeUpdated_negativeValues()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        const QString windowId = QStringLiteral("win-minsize-neg");

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);
        engine.windowOpened(windowId, screenName, 100, 50);
        QCoreApplication::processEvents();

        // Negative values should be clamped to 0 internally
        engine.windowMinSizeUpdated(windowId, -10, -20);
        QCoreApplication::processEvents();
    }

    void testWindowMinSizeUpdated_zeroRemovesEntry()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        const QString windowId = QStringLiteral("win-minsize-zero");

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);
        engine.windowOpened(windowId, screenName, 100, 50);
        QCoreApplication::processEvents();

        QSignalSpy tilingSpy(&engine, &AutotileEngine::tilingChanged);
        // Setting to 0,0 should remove the min-size entry and retile
        engine.windowMinSizeUpdated(windowId, 0, 0);
        QCoreApplication::processEvents();

        QVERIFY(tilingSpy.count() >= 1);
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Overflow window tests
    //
    // Without a ScreenManager, recalculateLayout can't produce zones (invalid
    // screen geometry). We exploit that it returns early WITHOUT clearing
    // existing zones — so we set zones manually on TilingState, then trigger
    // retile to exercise the overflow logic in applyTiling().
    // ═══════════════════════════════════════════════════════════════════════════

    void testOverflow_excessWindowsAutoFloated()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        // Start with high maxWindows so all 3 windows pass the gate check
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-1"), screenName);
        engine.windowOpened(QStringLiteral("win-2"), screenName);
        engine.windowOpened(QStringLiteral("win-3"), screenName);
        QCoreApplication::processEvents();

        TilingState* state = engine.stateForScreen(screenName);
        QVERIFY(state != nullptr);
        QCOMPARE(state->tiledWindowCount(), 3);

        // Simulate maxWindows decrease: set only 2 zones (as if recalculate
        // computed zones for min(3, 2) = 2 windows with valid screen geometry)
        engine.config()->maxWindows = 2;
        state->setCalculatedZones({QRect(0, 0, 500, 500), QRect(500, 0, 500, 500)});
        engine.retile(screenName);

        // Third window should be auto-floated (overflow)
        QCOMPARE(state->tiledWindowCount(), 2);
        QVERIFY(state->isFloating(QStringLiteral("win-3")));
    }

    void testOverflow_emitsFloatingSignal()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-a"), screenName);
        engine.windowOpened(QStringLiteral("win-b"), screenName);
        QCoreApplication::processEvents();

        TilingState* state = engine.stateForScreen(screenName);

        // Set 1 zone to simulate maxWindows=1
        engine.config()->maxWindows = 1;
        state->setCalculatedZones({QRect(0, 0, 1000, 1000)});

        QSignalSpy floatSpy(&engine, &AutotileEngine::windowFloatingChanged);
        engine.retile(screenName);

        // Should get a windowFloatingChanged(win-b, true, screen) signal
        bool foundOverflow = false;
        for (int i = 0; i < floatSpy.count(); ++i) {
            if (floatSpy.at(i).at(0).toString() == QStringLiteral("win-b")
                && floatSpy.at(i).at(1).toBool() == true) {
                foundOverflow = true;
                break;
            }
        }
        QVERIFY(foundOverflow);
    }

    void testOverflow_unfloatWhenRoomAvailable()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-1"), screenName);
        engine.windowOpened(QStringLiteral("win-2"), screenName);
        engine.windowOpened(QStringLiteral("win-3"), screenName);
        QCoreApplication::processEvents();

        TilingState* state = engine.stateForScreen(screenName);
        QCOMPARE(state->tiledWindowCount(), 3);

        // Overflow: set 2 zones to simulate maxWindows=2
        engine.config()->maxWindows = 2;
        state->setCalculatedZones({QRect(0, 0, 500, 500), QRect(500, 0, 500, 500)});
        engine.retile(screenName);
        QVERIFY(state->isFloating(QStringLiteral("win-3")));
        QCOMPARE(state->tiledWindowCount(), 2);

        // Close one tiled window — makes room for overflow
        engine.windowClosed(QStringLiteral("win-1"));
        QCoreApplication::processEvents();

        // win-3 should be auto-unfloated by unfloatOverflowIfRoom
        QVERIFY(!state->isFloating(QStringLiteral("win-3")));
    }

    void testOverflow_userFloatRemovesOverflowTracking()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-1"), screenName);
        engine.windowOpened(QStringLiteral("win-2"), screenName);
        QCoreApplication::processEvents();

        TilingState* state = engine.stateForScreen(screenName);

        // Overflow win-2 by setting 1 zone
        engine.config()->maxWindows = 1;
        state->setCalculatedZones({QRect(0, 0, 1000, 1000)});
        engine.retile(screenName);
        QVERIFY(state->isFloating(QStringLiteral("win-2")));

        // Increase maxWindows so unfloat doesn't immediately re-overflow
        engine.config()->maxWindows = 2;
        state->setCalculatedZones({QRect(0, 0, 500, 1000), QRect(500, 0, 500, 1000)});

        // Manually unfloat — removes overflow tracking (now user-controlled)
        engine.unfloatWindow(QStringLiteral("win-2"));

        // win-2 should be unfloated since maxWindows now accommodates it
        QVERIFY(!state->isFloating(QStringLiteral("win-2")));
        QCOMPARE(state->tiledWindowCount(), 2);
    }

    void testOverflow_screenRemovalCleansOverflow()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-1"), screenName);
        engine.windowOpened(QStringLiteral("win-2"), screenName);
        QCoreApplication::processEvents();

        TilingState* state = engine.stateForScreen(screenName);

        // Overflow win-2
        engine.config()->maxWindows = 1;
        state->setCalculatedZones({QRect(0, 0, 1000, 1000)});
        engine.retile(screenName);
        QVERIFY(state->isFloating(QStringLiteral("win-2")));

        // Remove screen from autotile
        engine.setAutotileScreens({});

        // Engine should have no autotile screens
        QVERIFY(!engine.isEnabled());
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Coverage gap tests (OverflowManager integration)
    // ═══════════════════════════════════════════════════════════════════════════

    void testOverflow_crossScreenMigration()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screen1 = QStringLiteral("Screen1");
        const QString screen2 = QStringLiteral("Screen2");
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screen1, screen2};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-1"), screen1);
        engine.windowOpened(QStringLiteral("win-2"), screen1);
        QCoreApplication::processEvents();

        TilingState* state1 = engine.stateForScreen(screen1);

        // Overflow win-2 on Screen1
        engine.config()->maxWindows = 1;
        state1->setCalculatedZones({QRect(0, 0, 1000, 1000)});
        engine.retile(screen1);
        QVERIFY(state1->isFloating(QStringLiteral("win-2")));

        // Simulate cross-screen focus (migration)
        engine.windowFocused(QStringLiteral("win-2"), screen2);
        QCoreApplication::processEvents();

        // win-2 should have been migrated: removed from Screen1
        QVERIFY(!state1->containsWindow(QStringLiteral("win-2")));
    }

    void testOverflow_backfillPriority()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-1"), screenName);
        engine.windowOpened(QStringLiteral("win-2"), screenName);
        engine.windowOpened(QStringLiteral("win-3"), screenName);
        QCoreApplication::processEvents();

        TilingState* state = engine.stateForScreen(screenName);
        QCOMPARE(state->tiledWindowCount(), 3);

        // Overflow: set 2 zones
        engine.config()->maxWindows = 2;
        state->setCalculatedZones({QRect(0, 0, 500, 500), QRect(500, 0, 500, 500)});
        engine.retile(screenName);
        QVERIFY(state->isFloating(QStringLiteral("win-3")));

        // Increase maxWindows — overflow recovery should bring win-3 back
        engine.config()->maxWindows = 3;
        state->setCalculatedZones({QRect(0, 0, 333, 500), QRect(333, 0, 333, 500), QRect(666, 0, 334, 500)});
        engine.retile(screenName);

        QVERIFY(!state->isFloating(QStringLiteral("win-3")));
    }

    void testOverflow_multipleUnfloat()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-1"), screenName);
        engine.windowOpened(QStringLiteral("win-2"), screenName);
        engine.windowOpened(QStringLiteral("win-3"), screenName);
        engine.windowOpened(QStringLiteral("win-4"), screenName);
        QCoreApplication::processEvents();

        TilingState* state = engine.stateForScreen(screenName);

        // Overflow 2 windows: only 2 zones
        engine.config()->maxWindows = 2;
        state->setCalculatedZones({QRect(0, 0, 500, 500), QRect(500, 0, 500, 500)});
        engine.retile(screenName);
        QCOMPARE(state->tiledWindowCount(), 2);

        // Increase to allow all 4 — both overflow windows should unfloat
        engine.config()->maxWindows = 4;
        state->setCalculatedZones({QRect(0, 0, 250, 500), QRect(250, 0, 250, 500),
                                   QRect(500, 0, 250, 500), QRect(750, 0, 250, 500)});

        QSignalSpy floatSpy(&engine, &AutotileEngine::windowFloatingChanged);
        engine.retile(screenName);

        // Count unfloat signals (floating=false)
        int unfloatCount = 0;
        for (int i = 0; i < floatSpy.count(); ++i) {
            if (!floatSpy.at(i).at(1).toBool()) {
                ++unfloatCount;
            }
        }
        QCOMPARE(unfloatCount, 2);
    }

    void testOverflow_userFloatClearsTracking()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-1"), screenName);
        engine.windowOpened(QStringLiteral("win-2"), screenName);
        QCoreApplication::processEvents();

        TilingState* state = engine.stateForScreen(screenName);

        // Overflow win-2
        engine.config()->maxWindows = 1;
        state->setCalculatedZones({QRect(0, 0, 1000, 1000)});
        engine.retile(screenName);
        QVERIFY(state->isFloating(QStringLiteral("win-2")));

        // Explicitly unfloat then re-float via user action to establish
        // user-intentional floating (floatWindow no-ops on already-floating windows)
        engine.config()->maxWindows = 2;
        state->setCalculatedZones({QRect(0, 0, 500, 1000), QRect(500, 0, 500, 1000)});
        engine.unfloatWindow(QStringLiteral("win-2"));  // clears overflow tracking
        engine.floatWindow(QStringLiteral("win-2"));    // now user-floated

        // Retile — win-2 should NOT auto-unfloat because it's user-floated
        engine.retile(screenName);
        QVERIFY(state->isFloating(QStringLiteral("win-2")));
    }

    void testOverflow_reentrantRetile()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-1"), screenName);
        engine.windowOpened(QStringLiteral("win-2"), screenName);
        QCoreApplication::processEvents();

        TilingState* state = engine.stateForScreen(screenName);

        // Overflow win-2
        engine.config()->maxWindows = 1;
        state->setCalculatedZones({QRect(0, 0, 1000, 1000)});

        // Retile should not crash on re-entrant call
        engine.retile(screenName);
        engine.retile(screenName);

        QVERIFY(state->isFloating(QStringLiteral("win-2")));
    }

    void testOverflow_multiScreenRemoval()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screen1 = QStringLiteral("Screen1");
        const QString screen2 = QStringLiteral("Screen2");
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screen1, screen2};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-1"), screen1);
        engine.windowOpened(QStringLiteral("win-2"), screen1);
        engine.windowOpened(QStringLiteral("win-3"), screen2);
        engine.windowOpened(QStringLiteral("win-4"), screen2);
        QCoreApplication::processEvents();

        TilingState* state1 = engine.stateForScreen(screen1);
        TilingState* state2 = engine.stateForScreen(screen2);

        // Overflow on both screens
        engine.config()->maxWindows = 1;
        state1->setCalculatedZones({QRect(0, 0, 1000, 1000)});
        state2->setCalculatedZones({QRect(0, 0, 1000, 1000)});
        engine.retile();
        QVERIFY(state1->isFloating(QStringLiteral("win-2")));
        QVERIFY(state2->isFloating(QStringLiteral("win-4")));

        // Remove Screen1 only — Screen2's overflow should be preserved
        engine.setAutotileScreens({screen2});
        QVERIFY(engine.isEnabled());

        // Screen2 state should still have win-4 floating
        TilingState* state2After = engine.stateForScreen(screen2);
        QVERIFY(state2After->isFloating(QStringLiteral("win-4")));
    }

    void testOverflow_perScreenMaxWindows()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-1"), screenName);
        engine.windowOpened(QStringLiteral("win-2"), screenName);
        engine.windowOpened(QStringLiteral("win-3"), screenName);
        QCoreApplication::processEvents();

        TilingState* state = engine.stateForScreen(screenName);
        QCOMPARE(state->tiledWindowCount(), 3);

        // Apply per-screen override to limit maxWindows to 2
        QVariantMap overrides;
        overrides[QStringLiteral("MaxWindows")] = 2;
        engine.applyPerScreenConfig(screenName, overrides);

        // Set 2 zones to match
        state->setCalculatedZones({QRect(0, 0, 500, 500), QRect(500, 0, 500, 500)});
        engine.retile(screenName);

        QCOMPARE(state->tiledWindowCount(), 2);
        QVERIFY(state->isFloating(QStringLiteral("win-3")));
    }

    void testOverflow_toggleFloatClearsTracking()
    {
        AutotileEngine engine(nullptr, nullptr, nullptr);
        const QString screenName = QStringLiteral("TestScreen");
        engine.config()->maxWindows = 10;

        QSet<QString> screens{screenName};
        engine.setAutotileScreens(screens);

        engine.windowOpened(QStringLiteral("win-1"), screenName);
        engine.windowOpened(QStringLiteral("win-2"), screenName);
        QCoreApplication::processEvents();

        TilingState* state = engine.stateForScreen(screenName);
        state->setFocusedWindow(QStringLiteral("win-2"));

        // Overflow win-2
        engine.config()->maxWindows = 1;
        state->setCalculatedZones({QRect(0, 0, 1000, 1000)});
        engine.retile(screenName);
        QVERIFY(state->isFloating(QStringLiteral("win-2")));

        // Toggle float on overflow window — should clear overflow status
        engine.toggleWindowFloat(QStringLiteral("win-2"), screenName);

        // After toggle, win-2 is now tiled (was floating -> toggled to tiled)
        // Increase maxWindows — win-2 should remain in its current state
        // (not re-tracked as overflow)
        engine.config()->maxWindows = 2;
        state->setCalculatedZones({QRect(0, 0, 500, 1000), QRect(500, 0, 500, 1000)});
        engine.retile(screenName);

        // win-2 should be tiled normally
        QVERIFY(!state->isFloating(QStringLiteral("win-2")));
        QCOMPARE(state->tiledWindowCount(), 2);
    }
};

QTEST_MAIN(TestAutotileEngine)
#include "test_autotile_engine.moc"