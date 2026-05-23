#ifndef FLOATINGPLATFORM_H
#define FLOATINGPLATFORM_H

#include "Block.h"
#include <QBrush>

// 單向平台：從下面可以穿過，但可以站在上面
class FloatingPlatform : public Block {
public:
    FloatingPlatform(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr)
        : Block(x, y, width, height)
    {
        setBrush(QBrush(Qt::blue));
        setPen(QPen(Qt::NoPen));
        setZValue(0);
        setFlag(QGraphicsItem::ItemIsMovable, false);
    }

    void setPlatformColor(const QBrush &b) { setBrush(b); }
};

#endif // FLOATINGPLATFORM_H
