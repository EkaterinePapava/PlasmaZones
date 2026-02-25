// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QHash>
#include <QRect>
#include <QRectF>
#include <QElapsedTimer>
#include <QtMath>

namespace KWin {
class EffectWindow;
class WindowPaintData;
}

namespace PlasmaZones {

/**
 * @brief Available easing curves for window animations
 *
 * Users can select any of these from the KCM. Custom curves
 * could be added by extending this enum and applyEasing().
 */
enum class EasingCurve : int {
    Linear = 0,
    OutQuad,        ///< Fast start, smooth deceleration (default legacy)
    OutCubic,       ///< Slightly more pronounced deceleration
    InOutCubic,     ///< Smooth acceleration then deceleration
    OutBack,        ///< Slight overshoot past target, then settle
    OutElastic,     ///< Spring-like bounce past target
    OutBounce,      ///< Ball-drop style bouncing settle
    InOutBack,      ///< Overshoot on both ends
    OutQuart,       ///< Sharper deceleration than cubic
    OutQuint,       ///< Very sharp deceleration
    OutExpo,        ///< Exponential deceleration — snappiest non-overshoot
    InOutQuad,      ///< Gentle symmetric acceleration/deceleration
    InOutElastic,   ///< Elastic spring on both ends
    Count           ///< Sentinel for validation
};

/**
 * @brief Animation data for window geometry transitions
 *
 * Stores the start/end geometry and progress for smooth window animations.
 */
struct WindowAnimation {
    QRectF startGeometry;   ///< Window geometry at animation start
    QRectF endGeometry;     ///< Target window geometry
    QElapsedTimer timer;    ///< Timer for animation progress calculation
    qreal duration = 150.0; ///< Animation duration in milliseconds (default 150ms)
    EasingCurve easing = EasingCurve::OutCubic; ///< Easing curve for this animation

    /// Check if the animation timer has been started
    bool isValid() const {
        return timer.isValid();
    }

    /// Apply the selected easing function to a linear time value (0.0-1.0)
    static qreal applyEasing(EasingCurve curve, qreal t);

    /// Calculate current progress (0.0 to 1.0) with the configured easing
    qreal progress() const {
        if (!timer.isValid()) {
            return 0.0;
        }
        qreal t = qMin(1.0, timer.elapsed() / duration);
        return applyEasing(easing, t);
    }

    /// Check if animation is complete
    bool isComplete() const {
        if (!timer.isValid()) {
            return true; // Invalid animation is considered complete
        }
        return timer.elapsed() >= duration;
    }

    /// Interpolate geometry based on current progress
    QRectF currentGeometry() const {
        qreal p = progress();
        return QRectF(
            startGeometry.x() + (endGeometry.x() - startGeometry.x()) * p,
            startGeometry.y() + (endGeometry.y() - startGeometry.y()) * p,
            startGeometry.width() + (endGeometry.width() - startGeometry.width()) * p,
            startGeometry.height() + (endGeometry.height() - startGeometry.height()) * p
        );
    }
};

/**
 * @brief Manages window geometry animations
 *
 * Tracks animation state, computes interpolated geometry, and determines
 * when animations are complete. The effect applies geometry immediately via
 * moveResize(), then visually interpolates from old to new using paint transforms.
 */
class WindowAnimator : public QObject
{
    Q_OBJECT

public:
    explicit WindowAnimator(QObject* parent = nullptr);

    // Configuration
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    void setDuration(qreal ms) { m_duration = ms; }
    qreal duration() const { return m_duration; }
    void setEasingCurve(EasingCurve curve) { m_easing = curve; }
    EasingCurve easingCurve() const { return m_easing; }
    void setMinDistance(int pixels) { m_minDistance = qMax(0, pixels); }
    int minDistance() const { return m_minDistance; }

    // Animation management
    bool hasActiveAnimations() const { return !m_animations.isEmpty(); }
    bool hasAnimation(KWin::EffectWindow* window) const;
    bool startAnimation(KWin::EffectWindow* window, const QRectF& startGeometry, const QRect& endGeometry);
    void removeAnimation(KWin::EffectWindow* window);
    void clear();

    // Animation state queries
    bool isAnimationComplete(KWin::EffectWindow* window) const;
    QRectF currentGeometry(KWin::EffectWindow* window) const;
    QRect finalGeometry(KWin::EffectWindow* window) const;

    // Paint helper - applies transform to paint data
    void applyTransform(KWin::EffectWindow* window, KWin::WindowPaintData& data);

    // Get the bounding rect covering the full animation path (start ∪ end geometry)
    // Used by the effect to request screen-region repaints that prevent ghost artifacts
    QRectF animationBounds(KWin::EffectWindow* window) const;

    // Check if window is already animating to the same target
    bool isAnimatingToTarget(KWin::EffectWindow* window, const QRect& targetGeometry) const;

    // Redirect animation to new target (for rapid geometry changes)
    void redirectAnimation(KWin::EffectWindow* window, const QRect& newTarget);

private:
    QHash<KWin::EffectWindow*, WindowAnimation> m_animations;
    bool m_enabled = true;
    qreal m_duration = 150.0;
    EasingCurve m_easing = EasingCurve::OutCubic;
    int m_minDistance = 0;
};

} // namespace PlasmaZones
