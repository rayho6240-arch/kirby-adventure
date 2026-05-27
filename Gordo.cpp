#include "Gordo.h"

// =========================================================
// 1. 初始化與建構子 (Initialization)
// =========================================================

/**
 * @brief Gordo 建構子
 * @param parent 傳遞給 QGraphicsPixmapItem 的父物件
 */
Gordo::Gordo(QGraphicsItem *parent) : Enemy(parent) {
    enemyType = "Gordo";

    // --- 預載入圖片 ---
    gordo0.load(":/Project2_Dataset/Image/Gordo/Gordo(0).png");
    gordo0 = gordo0.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    gordo1.load(":/Project2_Dataset/Image/Gordo/Gordo(1).png");
    gordo1 = gordo1.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // --- 設定初始圖片與物理參數 ---
    setPixmap(gordo0);
    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);

    // Gordo 原地待機，不移動
    vx = 0;
    vy = 0;
}

// =========================================================
// 2. 專屬更新邏輯 (Specific Update)
// =========================================================

/**
 * @brief Gordo 的每幀更新
 * @note Gordo 只進行動畫切換，不需要物理運算
 */
void Gordo::update() {
    // 死亡攔截
    if (isDead) return;

    // Gordo 不移動，只更新動畫
    updateSprite();
}

// =========================================================
// 3. 動畫更新 (Animation Update)
// =========================================================

/**
 * @brief 切換 Gordo 的靜止動畫
 */
void Gordo::updateSprite() {
    frameCounter++;

    // 每隔 ANIMATION_INTERVAL 幀切換一次圖片
    if ((frameCounter / ANIMATION_INTERVAL) % 2 == 0) {
        setPixmap(gordo0);
    } else {
        setPixmap(gordo1);
    }
}
