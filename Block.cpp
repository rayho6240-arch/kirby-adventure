#include "Block.h"
#include <QBrush>
#include <QPen>

Block::Block(qreal x, qreal y, qreal w, qreal h) {
    QPolygonF poly;
    poly << QPointF(0, 0) << QPointF(w, 0) << QPointF(w, h) << QPointF(0, h);
    setPolygon(poly);
    setPos(x, y);
    setBrush(QBrush(QColor(255, 0, 0, 100)));
    setPen(Qt::NoPen);
}

Block::Block(qreal x, qreal y, const QPolygonF& polygon){
    setPolygon(polygon);
    setPos(x, y);
    setBrush(QBrush(QColor(0, 0, 255, 100)));
    setPen(Qt::NoPen);
}