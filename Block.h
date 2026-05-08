#ifndef BLOCK_H
#define BLOCK_H

//一般地形與方塊的基底。

#include <QGraphicsRectItem> // 改繼承矩形

class Block : public QGraphicsRectItem {
public:


    // 定義一個特殊的 Type ID
    enum { Type = UserType + 1 };
    int type() const override { return Type; }
    // 傳入 x, y 座標，以及寬(w)高(h)
    Block(qreal x, qreal y, qreal w, qreal h);


};

#endif // BLOCK_H
