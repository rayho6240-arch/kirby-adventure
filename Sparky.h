#ifndef SPARKY_H
#define SPARKY_H

#include "Enemy.h"
#include <QGraphicsItem> // 確保有引入這個

class Sparky: public Enemy{
public:
    Sparky(QGraphicsItem *player = nullptr, QGraphicsItem *parent = nullptr);
    void update() override;
private:
    enum SparkState{
        IDLE,
        JUMPING
    };
    SparkState currentState;
    int stateTimer;
    int facingDirection;

    // 【新增】：用來儲存卡比的指標
    QGraphicsItem *targetPlayer;

};


#endif // SPARKY_H
