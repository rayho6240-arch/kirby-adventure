#include "Sparky.h"
#include <QPixmap>
#include <QDebug>
#include <cmath>  // 🌟 新增這行：為了使用算距離的 sqrt 函式

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

        // 載入放電特效圖
        attackImage1.load(":/Project2_Dataset/Image/Sparky/Sparky_attack_1.png");
        attackImage1 = attackImage1.scaled(spawnSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        attackImage2.load(":/Project2_Dataset/Image/Sparky/Sparky_attack_2.png");
        attackImage2 = attackImage2.scaled(spawnSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);


        // 2. 設定初始圖片
        setPixmap(leftImage1);
        setShapeMode(QGraphicsPixmapItem::BoundingRectShape); //關鍵修復，使他不會突然消失。

        // ... 其他初始化 ...
        currentState = IDLE;
        stateTimer = 0;
        facingDirection = -1;
        vx = 0;
        frameCounter = 0;

}
void Sparky::update(){
    if (isDead) return;

    // 1. 偵測是否正在被吸入並切換狀態
    if (this->isBeingInhaled) { 
        currentState = BEING_INHALED;
    } else if (currentState == BEING_INHALED) {
        currentState = IDLE;
        stateTimer = 0;
    }

    // 2. 分支處理：被吸入時 vs. 正常行動時
    if (currentState == BEING_INHALED) {
        // --- 【被吸入模式】 ---
        if (targetPlayer != nullptr) {
            qreal dist = qAbs(targetPlayer->x() - this->x());
            if (dist < 20) { 
                setIsDead(true);
                setVisible(false);
                return;
            }
        }
        // 這裡不要寫 vx = 0，也不要 stateTimer++，直接跳到最後跑物理
    } 
    else {
        // --- 【正常 AI 模式】 --- (原本所有的 AI 邏輯都縮進到這個 else 裡)
        stateTimer++;

        if (currentState == IDLE) {
            vx = 0; // 現在這行只會在沒被吸時執行，不會衝突了！

            if (stateTimer >= 60) {
                stateTimer = 0;
                double distance = 9999;

                if (targetPlayer != nullptr) {
                    double dx = targetPlayer->x() - this->x();
                    double dy = targetPlayer->y() - this->y();
                    distance = std::sqrt(dx * dx + dy * dy);

                    if (dx > 0) facingDirection = 1;
                    else facingDirection = -1;
                }

                if (distance <= 300) {
                    currentState = ATTACKING;
                } else {
                    currentState = JUMPING;
                    vy = -12;
                    vx = facingDirection * 3;
                }
            }
        }
        else if (currentState == JUMPING){
            if (vx > 0) facingDirection = 1;
            if (vx < 0) facingDirection = -1;

            if (vy >= 0 && isOnGround){
                currentState = IDLE;
                stateTimer = 0;
                vx = 0;
            }
        }
        else if (currentState == ATTACKING) {
            vx = 0;
            if (stateTimer >= 60) {
                currentState = IDLE;
                stateTimer = 0;
            }
        }
    }

    // 3. 統一執行物理與動畫
    if (!isDead){
        handlePhysics(60,60);
    }

    if (isDead) return;
    updateSprite();
}

void Sparky::updateSprite() {
    frameCounter++;
    //TODO:  😱 掙扎動畫：快速切換蓄力圖和普通圖，看起來像在發抖
    /*if (currentState == BEING_INHALED) {
        if ((frameCounter / 3) % 2 == 0) {
            setPixmap(attackImage1); 
        } else {
            setPixmap(leftImage1);
        }
    }*/

    if (currentState == ATTACKING) {
        // ==========================================
        // ⚡ 新增：攻擊階段細分邏輯 (利用 stateTimer)
        // 假設 ATTACKING 狀態總共持續 60 幀
        // ==========================================

        if (stateTimer < 30) {
            // 1. 蓄力階段 (前 0~29 幀)
            // 播放 attack_1 圖片。
            // (改成下方的閃爍寫法)
            //setPixmap(attackImage1); 
            // 蓄力時，每 5 幀閃爍一次 (在 attack_1 和 待機圖 之間切換)
            int chargeFrame = (frameCounter / 5) % 2;
            if (chargeFrame == 0) {
                setPixmap(attackImage1);
            } else {
                // 顯示待機圖
                setPixmap(facingDirection == -1 ? leftImage1 : rightImage1);
            }
        } 
        else {
            // 2. 放電階段 (第 30~59 幀)
            // 為了表現出強大的電流，我們讓它閃得超快 (2 幀換一次圖)
            int shockFrame = (frameCounter / 2) % 2; 

            if (shockFrame == 0) {
                setPixmap(attackImage2); // 顯示放電圖
            } else {
                // 這裡有兩個選擇：
                // A. 顯示原本的待機圖 (看起來像電流在閃爍)
                // B. 顯示一張完全透明的圖 (閃爍感更強，但可能會破圖，需謹慎)
                
                // 建議選擇 A，看起來比較穩：
                if (facingDirection == -1) setPixmap(leftImage1);
                else setPixmap(rightImage1);
            }
        }
    } 
    else {
        // 🏃 原本的移動動畫 (保持不變)
        int animFrame = (frameCounter / 10) % 2;
        if (facingDirection == -1) {
            setPixmap(animFrame == 0 ? leftImage1 : leftImage2);
        } else {
            setPixmap(animFrame == 0 ? rightImage1 : rightImage2);
        }
    }
}
