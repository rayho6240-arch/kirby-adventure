#ifndef HOTHEAD_H
#define HOTHEAD_H

#include "Enemy.h"
#include <QPixmap>
#include <QGraphicsItem>

/**
 * @brief HotHead 熱焰怪敵人類別
 * @details 具有巡邏、追蹤、蓄力與噴火狀態的敵人。
 * 會在地面左右跑動，偵測到玩家後停止並面朝玩家，接著蓄力、噴火，最後回到巡邏。
 */
class HotHead : public Enemy {
public:
    // ==========================================
    // 生命週期與核心介面 (Lifecycle & Interface)
    // ==========================================

    /**
     * @brief HotHead 建構子
     * @param player 玩家 (卡比) 的指標，用於距離偵測
     * @param parent 傳遞給 QGraphicsPixmapItem 的父物件
     */
    HotHead(QGraphicsItem *player = nullptr, QGraphicsItem *parent = nullptr);

    /**
     * @brief 每幀更新 HotHead 的行為
     */
    void update() override;

    /**
     * @brief 畫面更新方法，覆寫基底類別 draw()
     */
    void draw() override;

    /**
     * @brief 取得敵人類型
     */
    QString getEnemyType() const override { return "HotHead"; }

    /**
     * @brief 檢查 HotHead 是否正在噴火
     */
    bool isFiring() const { return currentState == FIRE; }

private:
    // ==========================================
    // 狀態機 (State Machine)
    // ==========================================
    enum HotHeadState {
        PATROL,
        CHASE,
        CHARGE,
        FIRE,
        COOLDOWN,
        BEING_INHALED
    };

    void updateSprite();
    void checkPlayerDistance();
    void checkAttackRange();
    void launchFireball();
    void updateFacingDirection();
    void updateFireEffect();
    void applyFireDamage();

    HotHeadState currentState = PATROL;
    enum AttackMode {
        ATTACK_NONE,
        ATTACK_PROJECTILE,
        ATTACK_FLAME
    };
    AttackMode currentAttackMode = ATTACK_NONE;
    bool hasLaunchedProjectile = false;
    float stateTimer = 0.0f;
    int facingDirection = -1; ///< -1 為左，1 為右
    int frameCounter = 0;

    QPixmap runLeft;
    QPixmap runRight;
    QPixmap attackLeft;
    QPixmap attackRight;
    QPixmap fire1;
    QPixmap fire2Left;
    QPixmap fire2Right;
    QPixmap fire3Left;
    QPixmap fire3Right;
    QGraphicsPixmapItem *fireEffect = nullptr;

    const float PATROL_SPEED = 2.0f;
    const float FLAME_RANGE = 120.0f;
    const float DETECT_RANGE = 500.0f;
    const float CHASE_DURATION = 12.0f;   ///< 檢測後停下並面朝玩家的時間
    const float CHARGE_DURATION = 30.0f;  ///< 蓄力持續 0.5 秒
    const float FIRE_DURATION = 120.0f;   ///< 噴火持續 2 秒
    const float COOLDOWN_DURATION = 30.0f;///< 噴火後短暫冷卻

    QGraphicsItem *targetPlayer;
};

#endif // HOTHEAD_H
