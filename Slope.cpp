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
    
    // 💡 加上一個微小的容錯常數，防止卡比在斜坡最左/最右端時因為浮點數誤差被判定離地
    const qreal EPSILON = 1.0; 

    int n = poly.size();
    for (int i = 0; i < n; ++i) {
        QPointF p1 = poly[i];
        QPointF p2 = poly[(i + 1) % n];

        qreal x1 = p1.x();
        qreal x2 = p2.x();

        if (qFuzzyCompare(x1, x2)) {
            continue; // 垂直邊不提供地表高度
        }

        // 🎯 修正點 1：放寬邊緣限制（減去/加上 EPSILON），確保卡比在剛踩上斜坡的那一像素也能被抓到
        if (localX < (qMin(x1, x2) - EPSILON) || localX > (qMax(x1, x2) + EPSILON)) {
            continue; 
        }

        // 🎯 修正點 2：防止除以 0 或是超出範圍的比例，並限制 t 在 0.0 到 1.0 之間
        qreal t = (localX - x1) / (x2 - x1);
        if (t < 0.0) t = 0.0;
        if (t > 1.0) t = 1.0;

        qreal yValue = p1.y() + t * (p2.y() - p1.y());
        if (yValue < bestY) {
            bestY = yValue;
        }
    }

    if (bestY == std::numeric_limits<qreal>::max()) {
        // 安全 fallback：如果真的抓不到，回傳斜坡實體最高點的 Scene Y
        return y();
    }

    // 🎯 修正點 3：安全防禦
    // 如果你的 Polygon 本身就是 Scene 絕對座標，請直接 return bestY;
    // 如果你的 Polygon 是相對於斜坡左上角的 Local 座標，才用 y() + bestY;
    // 這裡我們維持你原本的設計，但加上邊緣防禦後，它不會再輕易觸發錯誤的 fallback 了！
    return y() + bestY;
}