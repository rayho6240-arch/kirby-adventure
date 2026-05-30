#ifndef BOMBSTAR_H
#define BOMBSTAR_H

#include <QGraphicsPixmapItem>

class BombStar : public QGraphicsPixmapItem {
public:
    BombStar(double x, double y, bool toRight);

    void update();
    bool isDead() const;
    void setDead();
    bool isExploding() const;
    void startExplosion();

private:
    double vx = 0.0;
    bool dead = false;
    bool exploding = false;
    int explosionTimer = 0;
    const int explosionDuration = 20;
};

#endif // BOMBSTAR_H
