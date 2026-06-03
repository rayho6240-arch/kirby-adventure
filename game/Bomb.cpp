#include "Bomb.h"
#include "Kirby.h"

#include <QDebug>
#include <QPixmap>
#include <QtGlobal>
#include <cmath>

Bomb::Bomb(double x, double y, double vx, double vy, double groundY, Kirby *player)
    : vx(vx),
      vy(vy),
      groundY(groundY),
      player(player)
{
    QPixmap bombPixmap(":/Project2_Dataset/Image/bomb/bomb(1).png");
    if (!bombPixmap.isNull()) {
        setPixmap(bombPixmap.scaledToHeight(150, Qt::SmoothTransformation));
    }

    setShapeMode(QGraphicsPixmapItem::MaskShape);
    setZValue(3);
    setPos(x, y);
}

void Bomb::update()
{
    if (dead) return;

    if (beingInhaled) return;

    if (exploding) {
        if (player && !hasDamagedKirby && explosionRect().intersects(player->sceneBoundingRect())) {
            player->takeDamage(1);
            hasDamagedKirby = true;
        }

        explosionTimer--;
        if (explosionTimer <= 0) {
            dead = true;
        }
        return;
    }

    vy += gravity;
    setX(x() + vx);
    setY(y() + vy);

    if (player && collidesWithItem(player)) {
        qDebug() << "Bomb hit Kirby and exploded";
        player->takeDamage(1);
        hasDamagedKirby = true;
        explode(false);
        return;
    }

    if (y() >= floorY()) {
        setY(floorY());
        bounceCount++;
        if (bounceCount >= 3) {
            explode(true);
        } else {
            vy = -std::abs(vy) * bounceFactor;
            vx *= horizontalDamping;
            qDebug() << "Bomb bounced, count =" << bounceCount;
        }
    }
}

bool Bomb::isDead() const
{
    return dead;
}

bool Bomb::isExploding() const
{
    return exploding;
}

void Bomb::markInhaled()
{
    dead = true;
    beingInhaled = false;
}

void Bomb::startInhale()
{
    if (dead || exploding) return;

    beingInhaled = true;
    vx = 0.0;
    vy = 0.0;
    qDebug() << "Bomb start being inhaled";
}

bool Bomb::isBeingInhaled() const
{
    return beingInhaled;
}

void Bomb::moveToward(const QPointF &target, double speed)
{
    if (!beingInhaled || dead || exploding) return;

    QPointF center = sceneBoundingRect().center();
    QPointF delta = target - center;
    double length = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());

    if (length <= 1.0) return;

    QPointF step(delta.x() / length * speed, delta.y() / length * speed);
    setPos(pos() + step);
}

void Bomb::explode(bool damageKirby)
{
    if (exploding) return;

    exploding = true;
    explosionTimer = explosionDuration;
    vx = 0.0;
    vy = 0.0;

    QPixmap explosionPixmap(":/Project2_Dataset/Image/bomb/explore_(1).png");
    if (!explosionPixmap.isNull()) {
        setPixmap(explosionPixmap.scaledToHeight(200, Qt::SmoothTransformation));
    }

    if (damageKirby && player && explosionRect().intersects(player->sceneBoundingRect())) {
        player->takeDamage(1);
        hasDamagedKirby = true;
    }

    qDebug() << "Bomb exploded";
}

QRectF Bomb::explosionRect() const
{
    return QRectF(sceneBoundingRect().center().x() - 60.0,
                  sceneBoundingRect().center().y() - 60.0,
                  120.0,
                  120.0);
}

double Bomb::floorY() const
{
    return groundY - boundingRect().height();
}
