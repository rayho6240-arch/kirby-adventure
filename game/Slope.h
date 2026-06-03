#ifndef SLOPE_H
#define SLOPE_H

#include "Block.h"
#include <QPolygonF>

/**
 * @brief Slope 斜坡地形類別
 * @details 繼承自 Block，用於表示可沿著斜坡表面行走的地形。
 */
class Slope : public Block {
public:
    Slope(qreal x, qreal y, qreal w, qreal h);
    Slope(qreal x, qreal y, const QPolygonF &polygon);

    qreal getSurfaceY(qreal kirbyX) const;
};

#endif // SLOPE_H
