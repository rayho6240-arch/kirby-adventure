#ifndef SPARKY_H
#define SPARKY_H

#include "Enemy.h"
#include <QGraphicsItem> // 確保有引入這個
#include <QString>

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

    void updateSprite();
    int frameCounter = 0;


    // 【新增】：用來儲存所有預載入好、縮放好的圖片
    QPixmap leftImage1, leftImage2;
    QPixmap rightImage1, rightImage2;

    QPixmap attack1, attack2;


};

#endif // SPARKY_H
