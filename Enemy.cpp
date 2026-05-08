#include "Enemy.h"
#include "Block.h"
#include <QGraphicsScene>

Enemy::Enemy(QGraphicsItem *parent) : QGraphicsPixmapItem(parent) {
    vx = 0; vy = 0; gravity = 0.8; isOnGround = false;
}

void Enemy::handlePhysics(qreal width, qreal height) {
    qreal oldY = y();

    qreal nextX = x() + vx;

    // --- 1. 世界邊界檢查 (0 ~ 4860) ---
    // 如果快要跑出地圖，強制修正位置並轉向
    if (nextX < 0) {
        nextX = 0;
        vx = -vx; // 往右轉
    } else if (nextX > 4860 - width) {
        nextX = 4860 - width;
        vx = -vx; // 往左轉
    }
    setX(nextX);





    // --- 1. 水平移動與折返邏輯 ---
    setX(x() + vx);

    QList<QGraphicsItem *> itemsX = scene()->collidingItems(this);
    for (QGraphicsItem *item : itemsX) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block) {
            // 根據移動方向修正位置，防止卡進牆裡
            if (vx > 0) {
                // 往右撞牆，退回牆左邊
                setX(block->x() - width - 1);
            } else {
                // 往左撞牆，退回牆右邊
                setX(block->x() + block->boundingRect().width() + 1);
            }

            // [核心]：折返跑關鍵
            vx = -vx;
            break; // 處理一次碰撞即可，跳出迴圈
        }
    }

    // 2. 垂直移動 (重力)
    vy += gravity;
    setY(y() + vy);
    isOnGround = false;

    QList<QGraphicsItem *> itemsY = scene()->collidingItems(this);
    for (QGraphicsItem *item : itemsY) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block) {
            if (vy >= 0 && (oldY + height <= block->y() + 30)) {
                setY(block->y() - height);
                vy = 0;
                isOnGround = true;
                break;
            }
        }
    }
}
