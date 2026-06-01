#include "StarBullet.h"
#include "Block.h"
#include "Enemy.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>
#include <QPainterPath>
#include <QPixmap>
#include <QRectF>

StarBullet::StarBullet(qreal x, qreal y, bool toRight) {
    if (toRight) {
        setPixmap(QPixmap(":/Project2_Dataset/Image/Kirby_normal/kirby_attack_star_R(1).png").scaled(141.53, 100));
        vx = 15.0;
    } else {
        setPixmap(QPixmap(":/Project2_Dataset/Image/Kirby_normal/kirby_attack_star_L(1).png").scaled(141.53, 100));
        vx = -15.0;
    }

    setPos(x, y);
}

QPainterPath StarBullet::shape() const {
    QPainterPath path;
    const QRectF hitbox = boundingRect().adjusted(45.0, 35.0, -45.0, -35.0);
    path.addRect(hitbox);
    return path;
}

void StarBullet::update() {
    if (!scene() || !isVisible()) return;

    setX(x() + vx);

    if (x() < 0 || x() > 4860) {
        setVisible(false);
        return;
    }

    QList<QGraphicsItem *> hits = scene()->collidingItems(this);

    for (QGraphicsItem *item : hits) {
        if (item == this) continue;

        Enemy *enemy = dynamic_cast<Enemy *>(item);
        if (enemy && !enemy->getIsDead() && enemy->isVisible() && collidesWithItem(enemy)) {
            return;
        }
    }

    for (QGraphicsItem *item : hits) {
        if (item == this) continue;

        Block *block = dynamic_cast<Block *>(item);
        if (block) {
            setVisible(false);
            return;
        }
    }
}
