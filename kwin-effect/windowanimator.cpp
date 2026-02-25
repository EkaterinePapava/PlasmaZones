// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowanimator.h"

#include <effect/effectwindow.h>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcEffect)

namespace PlasmaZones {

// ═══════════════════════════════════════════════════════════════════════════════
// Easing curve implementations
// ═══════════════════════════════════════════════════════════════════════════════

static qreal outBounce(qreal t) {
    if (t < 1.0 / 2.75) {
        return 7.5625 * t * t;
    } else if (t < 2.0 / 2.75) {
        t -= 1.5 / 2.75;
        return 7.5625 * t * t + 0.75;
    } else if (t < 2.5 / 2.75) {
        t -= 2.25 / 2.75;
        return 7.5625 * t * t + 0.9375;
    } else {
        t -= 2.625 / 2.75;
        return 7.5625 * t * t + 0.984375;
    }
}

qreal WindowAnimation::applyEasing(EasingCurve curve, qreal t) {
    switch (curve) {
    case EasingCurve::Linear:
        return t;
    case EasingCurve::OutQuad:
        return 1.0 - (1.0 - t) * (1.0 - t);
    case EasingCurve::OutCubic:
        return 1.0 - qPow(1.0 - t, 3.0);
    case EasingCurve::InOutCubic:
        return t < 0.5
            ? 4.0 * t * t * t
            : 1.0 - qPow(-2.0 * t + 2.0, 3.0) / 2.0;
    case EasingCurve::OutBack: {
        constexpr qreal c1 = 1.70158;
        constexpr qreal c3 = c1 + 1.0;
        return 1.0 + c3 * qPow(t - 1.0, 3.0) + c1 * qPow(t - 1.0, 2.0);
    }
    case EasingCurve::OutElastic: {
        if (t <= 0.0) return 0.0;
        if (t >= 1.0) return 1.0;
        constexpr qreal c4 = (2.0 * M_PI) / 3.0;
        return qPow(2.0, -10.0 * t) * qSin((t * 10.0 - 0.75) * c4) + 1.0;
    }
    case EasingCurve::OutBounce:
        return outBounce(t);
    case EasingCurve::InOutBack: {
        constexpr qreal c1 = 1.70158;
        constexpr qreal c2 = c1 * 1.525;
        return t < 0.5
            ? (qPow(2.0 * t, 2.0) * ((c2 + 1.0) * 2.0 * t - c2)) / 2.0
            : (qPow(2.0 * t - 2.0, 2.0) * ((c2 + 1.0) * (t * 2.0 - 2.0) + c2) + 2.0) / 2.0;
    }
    case EasingCurve::OutQuart:
        return 1.0 - qPow(1.0 - t, 4.0);
    case EasingCurve::OutQuint:
        return 1.0 - qPow(1.0 - t, 5.0);
    case EasingCurve::OutExpo:
        return t >= 1.0 ? 1.0 : 1.0 - qPow(2.0, -10.0 * t);
    case EasingCurve::InOutQuad:
        return t < 0.5
            ? 2.0 * t * t
            : 1.0 - qPow(-2.0 * t + 2.0, 2.0) / 2.0;
    case EasingCurve::InOutElastic: {
        if (t <= 0.0) return 0.0;
        if (t >= 1.0) return 1.0;
        constexpr qreal c5 = (2.0 * M_PI) / 4.5;
        return t < 0.5
            ? -(qPow(2.0, 20.0 * t - 10.0) * qSin((20.0 * t - 11.125) * c5)) / 2.0
            :  (qPow(2.0, -20.0 * t + 10.0) * qSin((20.0 * t - 11.125) * c5)) / 2.0 + 1.0;
    }
    default:
        return t;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// WindowAnimator
// ═══════════════════════════════════════════════════════════════════════════════

WindowAnimator::WindowAnimator(QObject* parent)
    : QObject(parent)
{
}

bool WindowAnimator::hasAnimation(KWin::EffectWindow* window) const
{
    return m_animations.contains(window);
}

bool WindowAnimator::startAnimation(KWin::EffectWindow* window, const QRectF& startGeometry, const QRect& endGeometry)
{
    if (!window || !m_enabled) {
        return false;
    }

    // If geometry is the same, no animation needed
    if (startGeometry.toRect() == endGeometry) {
        return false;
    }

    // Skip animation if geometry change is below the minimum distance threshold
    if (m_minDistance > 0) {
        qreal maxDelta = qMax(
            qMax(qAbs(endGeometry.x() - startGeometry.x()),
                 qAbs(endGeometry.y() - startGeometry.y())),
            qMax(qAbs(endGeometry.width() - startGeometry.width()),
                 qAbs(endGeometry.height() - startGeometry.height()))
        );
        if (maxDelta < m_minDistance) {
            return false;
        }
    }

    WindowAnimation anim;
    anim.startGeometry = startGeometry;
    anim.endGeometry = QRectF(endGeometry);
    anim.duration = m_duration;
    anim.easing = m_easing;
    anim.timer.start();
    m_animations[window] = anim;

    // Request repaint to start animation
    window->addRepaintFull();

    qCDebug(lcEffect) << "Started animation from" << startGeometry << "to" << endGeometry
                      << "duration:" << m_duration << "easing:" << static_cast<int>(m_easing);
    return true;
}

void WindowAnimator::removeAnimation(KWin::EffectWindow* window)
{
    m_animations.remove(window);
}

void WindowAnimator::clear()
{
    m_animations.clear();
}

bool WindowAnimator::isAnimationComplete(KWin::EffectWindow* window) const
{
    auto it = m_animations.constFind(window);
    if (it == m_animations.constEnd()) {
        return true; // No animation = complete
    }
    return it->isComplete();
}

QRectF WindowAnimator::currentGeometry(KWin::EffectWindow* window) const
{
    auto it = m_animations.constFind(window);
    if (it == m_animations.constEnd()) {
        return window ? window->frameGeometry() : QRectF();
    }
    return it->currentGeometry();
}

QRect WindowAnimator::finalGeometry(KWin::EffectWindow* window) const
{
    auto it = m_animations.constFind(window);
    if (it == m_animations.constEnd()) {
        return QRect();
    }
    return it->endGeometry.toRect();
}

void WindowAnimator::applyTransform(KWin::EffectWindow* window, KWin::WindowPaintData& data)
{
    auto it = m_animations.find(window);
    if (it == m_animations.end()) {
        return;
    }

    const WindowAnimation& anim = *it;

    // Validate animation state
    if (!anim.isValid()) {
        m_animations.erase(it);
        return;
    }

    QRectF current = anim.currentGeometry();
    QRectF original = window->frameGeometry();

    // Calculate translation offset
    data += QPointF(current.x() - original.x(), current.y() - original.y());

    // Calculate scale factors with minimum threshold
    constexpr qreal MinDimension = 10.0;
    if (original.width() >= MinDimension && original.height() >= MinDimension) {
        data.setXScale(current.width() / original.width());
        data.setYScale(current.height() / original.height());
    }
}

QRectF WindowAnimator::animationBounds(KWin::EffectWindow* window) const
{
    auto it = m_animations.constFind(window);
    if (it == m_animations.constEnd()) {
        return QRectF();
    }
    QRectF bounds = it->startGeometry.united(it->endGeometry);

    // Overshooting easing curves (OutBack ~10%, OutElastic ~35%, InOutBack ~10%)
    // produce interpolated positions beyond the start/end bounding box.
    // Inflate along each animation axis so the repaint region covers the full
    // overshoot path and no ghost artifacts are left behind.
    if (it->easing == EasingCurve::OutBack
        || it->easing == EasingCurve::OutElastic
        || it->easing == EasingCurve::InOutBack
        || it->easing == EasingCurve::InOutElastic) {
        // 40% margin covers OutElastic's peak overshoot (~35% at t≈0.15)
        constexpr qreal overshoot = 0.4;
        const qreal dx = qAbs(it->endGeometry.x() - it->startGeometry.x()) * overshoot;
        const qreal dy = qAbs(it->endGeometry.y() - it->startGeometry.y()) * overshoot;
        const qreal dw = qAbs(it->endGeometry.width() - it->startGeometry.width()) * overshoot;
        const qreal dh = qAbs(it->endGeometry.height() - it->startGeometry.height()) * overshoot;
        bounds.adjust(-dx - dw, -dy - dh, dx + dw, dy + dh);
    }
    return bounds;
}

bool WindowAnimator::isAnimatingToTarget(KWin::EffectWindow* window, const QRect& targetGeometry) const
{
    auto it = m_animations.constFind(window);
    if (it == m_animations.constEnd()) {
        return false;
    }
    return it->endGeometry.toRect() == targetGeometry;
}

void WindowAnimator::redirectAnimation(KWin::EffectWindow* window, const QRect& newTarget)
{
    auto it = m_animations.find(window);
    if (it == m_animations.end()) {
        return;
    }

    // Start new animation from current interpolated position
    WindowAnimation anim;
    anim.startGeometry = it->currentGeometry();
    anim.endGeometry = QRectF(newTarget);
    anim.duration = m_duration;
    anim.easing = m_easing;
    anim.timer.start();
    *it = anim;

    qCDebug(lcEffect) << "Redirected animation from" << anim.startGeometry << "to" << newTarget;
}

} // namespace PlasmaZones
