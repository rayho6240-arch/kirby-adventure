#include "Slope.h"
#include <limits>
#include <QtGlobal>

Slope::Slope(qreal x, qreal y, qreal w, qreal h)
    : Block(x, y, w, h) {
}

Slope::Slope(qreal x, qreal y, const QPolygonF &polygon)
    : Block(x, y, polygon) {
}

qreal Slope::getSurfaceY(qreal kirbyX) const {
    qreal localX = kirbyX - x();
    const QPolygonF poly = polygon();
    qreal bestY = std::numeric_limits<qreal>::max();

    int n = poly.size();
    for (int i = 0; i < n; ++i) {
        QPointF p1 = poly[i];
        QPointF p2 = poly[(i + 1) % n];

        qreal x1 = p1.x();
        qreal x2 = p2.x();

        if (qFuzzyCompare(x1, x2)) {
            continue; // 垂直邊不提供地表高度
        }

        if (localX < qMin(x1, x2) || localX > qMax(x1, x2)) {
            continue; // 不在這段邊的水平範圍內
        }

        qreal t = (localX - x1) / (x2 - x1);
        qreal yValue = p1.y() + t * (p2.y() - p1.y());
        if (yValue < bestY) {
            bestY = yValue;
        }
    }

    if (bestY == std::numeric_limits<qreal>::max()) {
        // 如果傳入 X 不在坡面水平範圍內，就回傳斜坡物件頂部 Y 作為安全 fallback。
        return y();
    }

    return y() + bestY;
}
