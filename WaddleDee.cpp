#include "WaddleDee.h"

WaddleDee::WaddleDee(QGraphicsItem *parent) : Enemy(parent) {
    vx = -2.0; // 初始往左走

    // 架構師階段：先用紅色方塊，註解留給同學換圖
    QPixmap pix(60, 60);
    pix.fill(Qt::red);
    setPixmap(pix);
}

void WaddleDee::update() {
    // 直接調用父類處理好的物理邏輯
    // 傳入 60, 60 作為它的物理碰撞大小
    handlePhysics(60, 60);

    if (y() > 2000) { // 假設 y > 2000 代表掉下深淵
        // 做法 A: 刪除自己 (正式版做法)
        // this->deleteLater();

        // 做法 B: 重生 (Debug 階段好用，讓你不用重開遊戲)
        setPos(800, 500);
        vy = 0;
    }
}
