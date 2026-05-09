#include "Enemy.h"
#include "Block.h"
#include <QGraphicsScene>
#include <QList>

Enemy::Enemy(QGraphicsItem *parent) : QGraphicsPixmapItem(parent) {
    vx = 0; vy = 0; gravity = 0.8; isOnGround = false;
}





void Enemy::update() {
    // 所有的敵人在更新前都要檢查是否死了
    if (isDead) return;

    // 呼叫物理計算 (假設每個地面敵人都需要這個)
    handlePhysics(60, 60);
}








void Enemy::handlePhysics(qreal width, qreal height) {
    qreal oldY = y();
    // 1. 先計算下一個位置
    qreal nextX = x() + vx;

    // 2. 世界邊界檢查 (0 ~ 4860) - 架構師的安全鎖
    if (nextX < 0) {
        nextX = 0;
        vx = -vx; // 撞到地圖左牆轉向
    } else if (nextX > 4860 - width) {
        nextX = 4860 - width;
        vx = -vx; // 撞到地圖右牆轉向
    }

    // 3. 正式設定位置
    setX(nextX);

    // --- 以下是你原本的方塊碰撞邏輯 ---
    QList<QGraphicsItem *> itemsX = scene()->collidingItems(this);
    for (QGraphicsItem *item : itemsX) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block) {
            if (vx > 0) setX(block->x() - width - 1);
            else setX(block->x() + block->boundingRect().width() + 1);
            vx = -vx;
            break;
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
