#ifndef ENEMY_H
#define ENEMY_H
#include <QGraphicsPixmapItem>
#include <QObject>



//所有敵人的父親
//各種敵人如waddledee 會先繼承，再多寫自己的邏輯。

class Enemy : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    Enemy(QGraphicsItem *parent = nullptr);
    virtual void update() = 0; // 純虛擬函式，由子類實作"行為"

    qreal vx, vy;//從 private 改到 public 為了讓卡比可以 "吸動他 "


    void setIsDead(bool dead) { isDead = dead; } //定義好了
    bool getIsDead() const { return isDead; }


protected: // protect讓子類也能存取的物理變數

    qreal gravity;
    bool isOnGround;
    bool isDead = false;

    void handlePhysics(qreal width, qreal height);  // 通用的物理位移與碰撞處理
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
