#include "Enemy.h"
#include "Block.h"
#include <QGraphicsScene>
#include <QList>

// =========================================================
// 1. 初始化與建構子 (Initialization)
// =========================================================

/**
 * @brief Enemy 建構子
 * @param parent 傳遞給 QGraphicsPixmapItem 的父物件
 */
Enemy::Enemy(QGraphicsItem *parent) : QGraphicsPixmapItem(parent) {
    // 預設物理參數
    vx = 0;
    vy = 0;
    gravity = 0.8;
    isOnGround = false;
    // this->enemyAbility = Ability::SPARK; // <-- 標記這隻是電光怪
}

// =========================================================
// 2. 核心更新邏輯 (Core Update)
// =========================================================

/**
 * @brief 敵人的基礎更新邏輯
 * @note 如果在 .h 檔中宣告為 "= 0" (純虛擬函式)，子類別 (如 WaddleDee)
 * 依然必須覆寫這個函式。子類別可以在自己的 update() 裡面透過呼叫
 * `Enemy::update();` 來重複利用這段死亡檢查與預設物理邏輯。
 */
void Enemy::update() {
    // --- 1. 死亡攔截 ---
    if (isDead) return;

    // --- 2. 呼叫物理計算 ---
    // 假設每個地面敵人都需要這個基礎碰撞 (預設寬高 60x60)
    handlePhysics(100, 100);
}

// =========================================================
// 3. 通用物理與碰撞系統 (Physics & Collision System)
// =========================================================

/**
 * @brief 處理通用的物理位移、重力與碰撞判定
 * @param width 敵人的物理寬度
 * @param height 敵人的物理高度
 */
void Enemy::handlePhysics(qreal width, qreal height) {
    qreal oldY = y();

    // -------------------------------------------
    // --- 1. 水平移動與碰撞 (X 軸) ---
    // -------------------------------------------
    qreal nextX = x() + vx;

    // A. 世界邊界檢查 (0 ~ 4860) - 架構師的安全鎖
    if (nextX < 0) {
        nextX = 0;
        vx = -vx; // 撞到地圖最左邊牆壁，反彈轉向
    } else if (nextX > 4860 - width) {
        nextX = 4860 - width;
        vx = -vx; // 撞到地圖最右邊牆壁，反彈轉向
    }

    // 正式套用初步計算好的 X 座標
    setX(nextX);

    // B. 方塊地形碰撞判定 (撞牆反彈邏輯)
    QList<QGraphicsItem *> itemsX = scene()->collidingItems(this);
    for (QGraphicsItem *item : itemsX) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block) {
            // 如果撞到方塊，把位置推回方塊邊緣 (多退 1 像素避免黏住)
            if (vx > 0) {
                setX(block->x() - width - 1);
            } else {
                setX(block->x() + block->boundingRect().width() + 1);
            }

            // 敵人碰到牆壁的經典行為：掉頭走反方向
            vx = -vx;
            break; // 處理完一個碰撞就跳出，避免同時撞到兩個牆角產生錯亂
        }
    }

    // -------------------------------------------
    // --- 2. 垂直移動與碰撞 (Y 軸重力) ---
    // -------------------------------------------
    vy += gravity;    // 套用重力加速度
    setY(y() + vy);   // 更新 Y 座標
    isOnGround = false; // 預設為不在地上

    QList<QGraphicsItem *> itemsY = scene()->collidingItems(this);
    for (QGraphicsItem *item : itemsY) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block) {
            // 下落碰撞判定：如果正在往下掉，且原本腳的位置高於方塊
            if (vy >= 0 && (oldY + height <= block->y() + 30)) {
                setY(block->y() - height); // 讓腳剛好貼齊地板
                vy = 0;                    // 垂直速度歸零 (停止掉落)
                isOnGround = true;         // 標記為踩在地上
                break;
            }
        }
    }
}
