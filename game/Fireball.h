#ifndef FIREBALL_H
#define FIREBALL_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QTimer>

class Fireball : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    Fireball(qreal x, qreal y, int direction, QGraphicsItem *parent = nullptr);

private slots:
    void onMoveTimer();

private:
    int direction;
    qreal speed = 14.0;
    QTimer *moveTimer = nullptr;
};

#endif // FIREBALL_H
