#include "Sparky.h"
#include <QPixmap>

Sparky::Sparky(QGraphicsItem *player, QGraphicsItem *parent)
    : Enemy(parent), targetPlayer(player) {

    QPixmap pix(60, 60);
    pix.fill(Qt::red);
    setPixmap(pix);

    currentState = IDLE;
    stateTimer = 0;
    facingDirection = -1;
    vx = 0;
}

void Sparky::update(){
    stateTimer++;

    if (currentState == IDLE) {
        stateTimer++;
        vx = 0;

        if (stateTimer >= 60) {
            currentState = JUMPING;
            stateTimer = 0;
            vy = -12;

            // ==========================================
            // 【核心邏輯】：起跳前，先看卡比在哪裡！
            // ==========================================
            if (targetPlayer != nullptr) {
                // 如果卡比的中心點，在我的中心點右邊
                if (targetPlayer->x() > this->x()) {
                    facingDirection = 1;  // 決定往右跳
                } else {
                    facingDirection = -1; // 決定往左跳
                }
            }

            vx = facingDirection * 3;
            //TODO: 根據面對方向切換圖片
        }
    }

    else if (currentState == JUMPING){
        if (vx>0) facingDirection=1;
        if (vx<0) facingDirection=-1;

        if(vy>=0&&isOnGround){
            currentState=IDLE;
            stateTimer=0;
            vx=0;

            //TODO 換回IDLE圖
        }
    }


    if (!isDead){
        handlePhysics(60,60);
    }
};


