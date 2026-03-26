// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DwindleMemoryAlgorithm.h"
#include "DwindleAlgorithm.h"
#include "../AlgorithmRegistry.h"
#include "../SplitTree.h"
#include "../TilingState.h"
#include "pz_i18n.h"

namespace PlasmaZones {

// Self-registration: Dwindle Memory provides persistent split tracking (priority 45)
namespace {
AlgorithmRegistrar<DwindleMemoryAlgorithm> s_dwindleMemoryRegistrar(DBus::AutotileAlgorithm::DwindleMemory, 45);
}

DwindleMemoryAlgorithm::DwindleMemoryAlgorithm(QObject* parent)
    : TilingAlgorithm(parent)
{
}

QString DwindleMemoryAlgorithm::name() const
{
    return PzI18n::tr("Dwindle (Memory)");
}

QString DwindleMemoryAlgorithm::description() const
{
    return PzI18n::tr("Remembers split positions — resize one split without affecting others");
}

QString DwindleMemoryAlgorithm::icon() const noexcept
{
    return QStringLiteral("view-grid-symbolic");
}

QVector<QRect> DwindleMemoryAlgorithm::calculateZones(const TilingParams& params) const
{
    const int windowCount = params.windowCount;
    const auto& screenGeometry = params.screenGeometry;
    const auto& outerGaps = params.outerGaps;

    QVector<QRect> zones;

    if (windowCount <= 0 || !screenGeometry.isValid() || !params.state) {
        return zones;
    }

    const QRect area = innerRect(screenGeometry, outerGaps);

    // Single window takes full available area
    if (windowCount == 1) {
        zones.append(area);
        return zones;
    }

    // Use persistent split tree if available and leaf count matches
    SplitTree* tree = params.state->splitTree();

    // Lazy tree creation: build tree from current window order on first use.
    // const_cast is acceptable here — the tree is lazy-initialized cache state,
    // not a logical mutation. The algorithm is const but needs to seed the tree
    // so that subsequent resize operations have a structure to work with.
    if (!tree && windowCount > 1) {
        auto newTree = std::make_unique<SplitTree>();
        const QStringList tiledWindows = params.state->tiledWindows();
        for (const QString& windowId : tiledWindows) {
            newTree->insertAtEnd(windowId);
        }
        const_cast<TilingState*>(params.state)->setSplitTree(std::move(newTree));
        tree = params.state->splitTree();
    }

    if (tree && tree->leafCount() == windowCount) {
        return tree->applyGeometry(area, params.innerGap);
    }

    // Fallback: count mismatch — behave like stateless dwindle
    return calculateStatelessFallback(params, area);
}

QVector<QRect> DwindleMemoryAlgorithm::calculateStatelessFallback(const TilingParams& params, const QRect& area) const
{
    Q_UNUSED(area)
    // Delegate to stateless DwindleAlgorithm — avoids duplicating 77 lines
    static DwindleAlgorithm s_fallback;
    return s_fallback.calculateZones(params);
}

} // namespace PlasmaZones
