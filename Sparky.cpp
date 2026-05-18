#include "Sparky.h"
#include <QPixmap>
#include <QDebug>

Sparky::Sparky(QGraphicsItem *player, QGraphicsItem *parent)
    : Enemy(parent), targetPlayer(player) {



    // 定義一個公用的縮放大小，確保跟物理 handlePhysics 對齊
        QSize spawnSize(60, 60);

        // 載入並縮放左邊圖 1
        leftImage1.load(":/Project2_Dataset/Image/Sparky/Sparky_left_1.png");
        leftImage1 = leftImage1.scaled(spawnSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        // 載入並縮放左邊圖 2
        leftImage2.load(":/Project2_Dataset/Image/Sparky/Sparky_left_2.png");
        leftImage2 = leftImage2.scaled(spawnSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        // 載入並縮放右邊圖 1 (注意你的檔名拼錯 ritht)
        rightImage1.load(":/Project2_Dataset/Image/Sparky/Sparky_ritht_1.png");
        rightImage1 = rightImage1.scaled(spawnSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        // 載入並縮放右邊圖 2
        rightImage2.load(":/Project2_Dataset/Image/Sparky/Sparky_ritht_2.png");
        rightImage2 = rightImage2.scaled(spawnSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        // 🛡️ 防護罩：檢查圖片是否真的載入成功 (如果路徑錯了這裡會印出錯誤)
        if (leftImage1.isNull()) qDebug() << "❌ 警告：Sparky 左側圖片載入失敗，請檢查資源路徑！";

        // 2. 設定初始圖片
        setPixmap(leftImage1);

        // ... 其他初始化 ...
        currentState = IDLE;
        stateTimer = 0;
        facingDirection = -1;
        vx = 0;
        frameCounter = 0;

}

void Sparky::update(){
    stateTimer++;

    if (currentState == IDLE) {
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
        }
    }

    else if (currentState == JUMPING){
        if (vx>0) facingDirection=1;
        if (vx<0) facingDirection=-1;

        if(vy>=0&&isOnGround){
            currentState=IDLE;
            stateTimer=0;
            vx=0;
        }
    }


    if (!isDead){
        handlePhysics(60,60);
    }

    updateSprite();
};


void Sparky::updateSprite() {
    // 1. 讓動畫計數器持續增加
    frameCounter++;

    // 2. 計算動畫格數 (算出 0 或 1)
    int animFrame = (frameCounter / 10) % 2;

    // 3. 直接從記憶體拿取你在建構子已經準備好的圖片！不需要再讀取硬碟了！
    if (facingDirection == -1) {
        // 面朝左
        if (animFrame == 0) {
            setPixmap(leftImage1);
        } else {
            setPixmap(leftImage2);
        }
    } else {
        // 面朝右
        if (animFrame == 0) {
            setPixmap(rightImage1);
        } else {
            setPixmap(rightImage2);
        }
    }
}


