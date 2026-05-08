#ifndef KIRBY_H
#define KIRBY_H

#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QString>
#include <QImage>
#include "Enemy.h"

// 讓 Kirby 繼承 QGraphicsPixmapItem，這樣他自己就是一個可以顯示在場景上的演員
class Kirby : public QGraphicsPixmapItem {
public:
    Kirby();

    // --- 動作指令 (提供給 MainWindow 呼叫) ---
    void jump();
    void fly();
    void startInhaling();
    void stopInhaling();
    void setHorizontalVelocity(qreal v);
    void setDashing(bool dashing);
    void setDown(bool down);


    // --- 核心更新邏輯 ---
    void update();


    //卡比面前畫一個「看不見的長方形」，只要敵人在裡面，就會受到吸引力。
    void processInhale(QList<Enemy*> &enemies); // 接收敵人清單進行處理


    // 取得卡比目前的 vx，供 MainWindow 判斷左右方向用
    qreal getVx() const { return vx; }

private:
    // 卡比自己的變數
    qreal vx = 0;
    qreal vy = 0;
    const qreal gravity = 0.8;
    const qreal DASH_SPEED = 12;

    bool isFacingRight = true;
    bool isDown = false;
    bool isFlying = false;
    bool isDashing = false;
    bool isInhaling = false;
    bool isOnGround = false;  // 新增這行：記錄卡比是否確實踩在地上


    int frameCounter = 0;
    int flapCounter = 0;

    // 換圖邏輯
    void updateSprite();


};

#endif // KIRBY_H
