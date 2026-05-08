#ifndef ENEMY_H
#define ENEMY_H

#include <QGraphicsPixmapItem>
#include <QObject>

class Enemy : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    Enemy(QGraphicsItem *parent = nullptr);
    virtual void update() = 0; // 純虛擬函式，強迫子類實作行為

protected:
    // 讓子類也能存取的物理變數
    qreal vx, vy;
    qreal gravity;
    bool isOnGround;

    // 通用的物理位移與碰撞處理
    void handlePhysics(qreal width, qreal height);
};

#endif
