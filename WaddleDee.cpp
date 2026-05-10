#include "WaddleDee.h"

// =========================================================
// 1. 初始化與建構子 (Initialization)
// =========================================================

/**
 * @brief WaddleDee 建構子
 * @note 冒號後面的 ": Enemy(parent)" 是 C++ 的「成員初始化列表」，
 * 負責在 WaddleDee 誕生前，先把爸爸 (Enemy) 準備好並把 parent 傳給他。
 */
WaddleDee::WaddleDee(QGraphicsItem *parent) : Enemy(parent) {
    vx = -2.0; // 初始給予往左走的速度

    // --- 簡易，預設外觀 ---
    // 先用紅色方塊代替，方便測試物理碰撞。留給同學換圖。
    QPixmap pix(60, 60);
    pix.fill(Qt::red);
    setPixmap(pix);
}

// =========================================================
// 2. 專屬更新邏輯 (Specific Update)
// =========================================================

/**
 * @brief WaddleDee 的每幀更新
 */
void WaddleDee::update() {
    // --- 1. 執行基底邏輯與死亡攔截 ---
    // [重點]：先呼叫父類別的 update，裡面已經包含了 if(isDead) 的檢查與基礎物理運算
    Enemy::update();

    // 如果敵人在 Enemy::update() 裡被判定死亡，子類別這裡也必須 return，
    // 避免執行後續 WaddleDee 獨有的行為（例如換動畫或掉落深淵判定）
    if (isDead) return;

    // --- 2. 物理邏輯 (⚠️ 注意重複呼叫問題) ---
    // [修正建議]：因為你在 Enemy.cpp 的 update() 裡面已經呼叫過 handlePhysics(60, 60) 了，
    // 這裡如果再呼叫一次，WaddleDee 每一幀會移動兩次、重力也會加倍！
    // 建議將這行註解掉，完全交給 Enemy::update() 去處理即可。
    // handlePhysics(60, 60);

    // --- 3. WaddleDee 專屬行為：深淵防護 ---
    if (y() > 2000) { // 假設 y > 2000 代表掉下深淵

        // 做法 A: 刪除自己 (正式版做法，釋放記憶體)
        // this->deleteLater();

        // 做法 B: 重生 (Debug 階段好用，讓你不用一直重開遊戲)
        setPos(800, 500);
        vy = 0; // 重生時要把掉落累積的重力加速度歸零，否則會一秒閃現墜落
    }

    // --- 4. 專屬動畫切換 (預留) ---
    // TODO: 未來這裡可以加上 updateSprite()，用來根據 vx 的正負值切換左右走路的圖片
}
