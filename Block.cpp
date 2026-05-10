#include "Block.h"
#include <QBrush>
#include <QPen>

Block::Block(qreal x, qreal y, qreal w, qreal h) {
    // 矩形本身的大小從 0, 0 開始畫，寬為 w，高為 h
    setRect(0, 0, w, h);

    //把整個方塊物件放到場景裡的 (x, y) 位置
    setPos(x, y);

    // 設定半透明紅色方便除錯
    setBrush(QBrush(QColor(255, 0, 0, 100)));
    setPen(Qt::NoPen);
}
