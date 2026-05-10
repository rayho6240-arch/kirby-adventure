#ifndef STARBULLET_H
#define STARBULLET_H

// ==========================================
// 1. Qt 內建函式庫引入
// ==========================================
#include <QGraphicsPixmapItem>
#include <QObject>

/**
 * @brief StarBullet 星星子彈類別
 * @details 卡比吸入敵人或方塊後，按下攻擊鍵「吐出來的星星子彈」。
 * 負責處理子彈的直線飛行、物理速度，以及打到敵人或牆壁時的判定。
 */
class StarBullet : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT // <--- 雖然目前還沒用到 signal/slot，但保留著有助於未來擴充 (例如：打到牆壁時發出爆炸音效的 signal)

public:
    // ==========================================
    // 2. 生命週期與核心介面 (Lifecycle & Interface)
    // ==========================================

    /**
     * @brief StarBullet 建構子
     * @param x 子彈生成的初始 X 座標 (通常為卡比的嘴巴位置)
     * @param y 子彈生成的初始 Y 座標
     * @param toRight 射擊方向：true 表示朝右，false 表示朝左
     */
    StarBullet(qreal x, qreal y, bool toRight);

    /**
     * @brief 子彈每幀的更新邏輯
     * @details 由 MainWindow 的主遊戲迴圈 (Game Loop) 統一呼叫。
     * 負責更新子彈座標，並掃描是否撞到牆壁 (Block) 或敵人 (Enemy)。
     */
    void update();

private:
    // ==========================================
    // 3. 物理與狀態變數 (Physics & State)
    // ==========================================

    qreal vx; ///< 子彈的水平飛行速度 (像素/幀)
};

#endif // STARBULLET_H
