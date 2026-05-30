#include "BombStar.h"
#include "Block.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>
#include <QPixmap>

BombStar::BombStar(double x, double y, bool toRight)
{
    const QString path = toRight
                             ? ":/Project2_Dataset/Image/bomb/bombStar_R.png"
                             : ":/Project2_Dataset/Image/bomb/bombStar_L.png";
    QPixmap pix(path);
    if (!pix.isNull()) {
        setPixmap(pix.scaledToHeight(80, Qt::SmoothTransformation));
    }

    setShapeMode(QGraphicsPixmapItem::MaskShape);
    setZValue(4);
    setPos(x, y);
    vx = toRight ? 16.0 : -16.0;
}

void BombStar::update()
{
    if (dead) return;

    setX(x() + vx);

    if (scene()) {
        if (x() < scene()->sceneRect().left() || x() > scene()->sceneRect().right()) {
            dead = true;
            return;
        }

        QList<QGraphicsItem *> colliding = scene()->collidingItems(this);
        for (QGraphicsItem *item : colliding) {
            if (item == this) continue;

            if (dynamic_cast<Block *>(item)) {
                dead = true;
                return;
            }
        }
    }
}

bool BombStar::isDead() const
{
    return dead;
}

void BombStar::setDead()
{
    dead = true;
}
