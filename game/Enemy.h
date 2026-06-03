#ifndef ENEMY_H
#define ENEMY_H

// ==========================================
// 1. Qt 內建函式庫引入
// ==========================================
#include <QGraphicsPixmapItem>
#include <QObject>

/**
 * @brief Enemy 所有敵人的抽象基底類別 (Base Class)
 * @details 統一定義敵人的物理運算、生命週期與基本屬性。
 * 各種具體敵人（如 Waddle Dee）必須繼承此類別，並實作自己的 update() 邏輯。
 */
class Enemy : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    // ==========================================
    // 2. 生命週期與核心介面 (Lifecycle & Interface)
    // ==========================================
    Enemy(QGraphicsItem *parent = nullptr);

    /**
     * @brief 每幀更新邏輯 (純虛擬函式 Pure Virtual Function)
     * @note "= 0" 代表 Enemy 本身不實作這個函式，而是「強制規定」所有繼承它的子類別
     * (如 WaddleDee) 都必須自己寫一個 update()。這讓 MainWindow 可以無腦呼叫 e->update()。
     */
    virtual void update() = 0;
    virtual void draw() {}

    // ==========================================
    // 3. 物理與互動開放變數 (Public Physics)
    // ==========================================
    // @note 故意設為 public，是為了讓 Kirby::processInhale()
    // 可以直接讀寫它們，從外部給予敵人一個被吸入的強大加速度。
    qreal vx = 0;
    qreal vy = 0;
    void setIsBeingInhaled(bool a){isInhaled = a;}
    // ==========================================
    // 4. 狀態管理 (State Flags)
    // ==========================================
    /**
     * @brief 設定敵人死亡狀態
     * @param dead 若為 true，代表敵人已被吃掉或擊敗，子類別的 update 應停止運作
     */
    void setIsDead(bool dead) { isDead = dead; }

    /**
     * @brief 取得敵人是否已死亡
     */
    bool getIsDead() const { return isDead; }




    virtual QString getEnemyType() const { return "Normal"; }

protected:
    // ==========================================
    // 5. 內部受保護的變數與函式 (Protected Members)
    // ==========================================
    // @note 使用 protected 讓繼承的子類別 (如 WaddleDee) 可以像使用自己的變數一樣直接讀取，
    // 但外部的類別 (如 MainWindow) 無法隨意竄改這些核心物理數值。

    qreal gravity = 0.8;      ///< 共用重力參數
    bool isOnGround = false;  ///< 是否踩在實體方塊上
    bool isDead = false;      ///< 死亡判定旗標
    bool isInhaled=false;

    /**
     * @brief 通用的物理位移與碰撞處理 (AABB 碰撞)
     * @param width 該敵人的物理碰撞箱寬度
     * @param height 該敵人的物理碰撞箱高度
     * @details 將複雜的地形判斷寫在這裡，子類別只需在自己的 update() 裡呼叫此函式，
     * 就能自動擁有「掉落地面」與「撞牆停止」的能力。
     */
    void handlePhysics(qreal width, qreal height);



    QString enemyType = "Normal"; // 預設值
};


#endif // ENEMY_H




//後續為了提升效能
//目前只是用 setVisible(false) 讓敵人消失，但這三個紅方塊其實還「躲」在後台默默地跑物理運算。
    /*後續
    if (shouldSwallow) {
    e->setVisible(false);
    e->setIsDead(true); // 假設你在 Enemy 裡有這個變數
    // 正式開發時，我們會在這裡把 e 從 enemyList 移除並 delete
    }*/
