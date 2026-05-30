#ifndef BOMB_H
#define BOMB_H

#include <QGraphicsPixmapItem>
#include <QRectF>

class Kirby;

class Bomb : public QGraphicsPixmapItem {
public:
    Bomb(double x, double y, double vx, double vy, double groundY, Kirby *player);

    void update();
    bool isDead() const;
    bool isExploding() const;
    void markInhaled();

private:
    void explode(bool damageKirby);
    QRectF explosionRect() const;
    double floorY() const;

    double vx = 0.0;
    double vy = 0.0;
    double gravity = 0.6;
    double groundY = 0.0;
    double bounceFactor = 0.65;
    double horizontalDamping = 0.9;

    int bounceCount = 0;
    int explosionTimer = 0;
    const int explosionDuration = 20;
    bool exploding = false;
    bool dead = false;
    bool hasDamagedKirby = false;

    Kirby *player = nullptr;
};

#endif // BOMB_H
