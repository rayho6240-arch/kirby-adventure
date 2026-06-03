#ifndef GORDO_H
#define GORDO_H

#include "Enemy.h"
#include <QPixmap>

/**
 * @brief Gordo 古多敵人類別
 * @details 原地待機型敵人，會在兩個靜止姿態之間切換動畫。
 */
class Gordo : public Enemy {
public:
    // ==========================================
    // 生命週期與核心介面 (Lifecycle & Interface)
    // ==========================================
    Gordo(QGraphicsItem *parent = nullptr);

    /**
     * @brief 實作 Gordo 專屬的每幀更新邏輯
     * @details Gordo 原地不動，只進行動畫切換
     */
    void update() override;

    /**
     * @brief 取得敵人類型
     */
    QString getEnemyType() const override { return "Gordo"; }

private:
    // ==========================================
    // 動畫與狀態 (Animation & State)
    // ==========================================
    void updateSprite();

    QPixmap gordo0;  ///< Gordo(0).png 靜止姿態 1
    QPixmap gordo1;  ///< Gordo(1).png 靜止姿態 2
    int frameCounter = 0;

    const int ANIMATION_INTERVAL = 30; ///< 每隔多少幀切換一次動畫
};

#endif // GORDO_H
