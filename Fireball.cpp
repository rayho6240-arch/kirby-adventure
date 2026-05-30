#include "Fireball.h"
#include "Kirby.h"
#include "Block.h"
#include "Enemy.h"

#include <QPixmap>
#include <QGraphicsScene>
#include <QList>
#include <QTimer>

Fireball::Fireball(qreal x, qreal y, int direction, QGraphicsItem *parent)
        : QObject(nullptr), QGraphicsPixmapItem(parent), direction(direction) 
{
    QPixmap pix(":/Project2_Dataset/Image/Hot Head/Hot_head_fire(1).png");
    pix = pix.scaled(40, 40, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    setPixmap(pix);
    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    setZValue(2);
    setPos(x, y);

    moveTimer = new QTimer(this);
    moveTimer->setInterval(16);
    connect(moveTimer, &QTimer::timeout, this, &Fireball::onMoveTimer);
    moveTimer->start();
}

void Fireball::onMoveTimer() {
    if (!scene() || !isVisible()) {
        deleteLater();
        return;
    }

    setX(x() + direction * speed);

    if (x() < 0 || x() > 4860) {
        setVisible(false);
        deleteLater();
        return;
    }

    QList<QGraphicsItem *> colliding = scene()->collidingItems(this);
    for (QGraphicsItem *item : colliding) {
        if (item == this) continue;

        Kirby *kirby = dynamic_cast<Kirby *>(item);
        if (kirby) {
            kirby->takeDamage(1);
            setVisible(false);
            deleteLater();
            return;
        }

        Block *block = dynamic_cast<Block *>(item);
        if (block) {
            setVisible(false);
            deleteLater();
            return;
        }
/*
        Enemy *enemy = dynamic_cast<Enemy *>(item);
        if (enemy) {
            // 不對自己的敵人造成傷害，僅避免火球穿透其他敵人
            setVisible(false);
            deleteLater();
            return;
        }*/
    }
}
