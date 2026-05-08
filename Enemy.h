#ifndef ENEMY_H
#define ENEMY_H

#include <QGraphicsPixmapItem>
#include <QObject>

class Enemy : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    Enemy(QGraphicsItem *parent = nullptr);
    virtual void update() = 0; // 純虛擬函式，強迫子類實作行為

    qreal vx, vy;//從private改到public 為了讓卡筆可以吸動他

protected:
    // 讓子類也能存取的物理變數

    qreal gravity;
    bool isOnGround;

    // 通用的物理位移與碰撞處理
    void handlePhysics(qreal width, qreal height);
};

#endif


//後續為了提升效能
//目前只是用 setVisible(false) 讓敵人消失，但這三個紅方塊其實還「躲」在後台默默地跑物理運算。
    /*後續
    if (shouldSwallow) {
    e->setVisible(false);
    e->setIsDead(true); // 假設你在 Enemy 裡有這個變數
    // 正式開發時，我們會在這裡把 e 從 enemyList 移除並 delete
    }*/
