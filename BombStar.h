#ifndef BOMBSTAR_H
#define BOMBSTAR_H

#include <QGraphicsPixmapItem>

class BombStar : public QGraphicsPixmapItem {
public:
    BombStar(double x, double y, bool toRight);

    void update();
    bool isDead() const;
    void setDead();

private:
    double vx = 0.0;
    bool dead = false;
};

#endif // BOMBSTAR_H
