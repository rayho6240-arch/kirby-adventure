#include "StarBullet.h"
#include "Block.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
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

void StarBullet::update() {
    if (!scene() || !isVisible()) return;

    const QRectF previousHitbox = sceneBoundingRect().adjusted(15.0, 15.0, -15.0, -15.0);

    setX(x() + vx);

    if (x() < 0 || x() > 4860) {
        setVisible(false);
        return;
    }

    const QRectF currentHitbox = sceneBoundingRect().adjusted(15.0, 15.0, -15.0, -15.0);
    const QRectF sweptHitbox = previousHitbox.united(currentHitbox);

    for (QGraphicsItem *item : scene()->items(sweptHitbox, Qt::IntersectsItemBoundingRect)) {
        if (item == this) continue;

        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block) {
            setVisible(false);
            return;
        }
    }
}
