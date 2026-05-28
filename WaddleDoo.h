#ifndef WADDLEDOO_H
#define WADDLEDOO_H

#include "Enemy.h"
#include <QPixmap>
#include <QPainterPath>

/**
 * @brief WaddleDoo 光束鞭敵人
 * @details 2D 橫向捲軸敵人，會在地面巡邏並發動一段由上至下的光束鞭攻擊。
 *          為了解決圖片透明區域的誤判碰撞，本類別會覆寫 shape()，
 *          使用多重實體碰撞區塊來匹配身體與光束部分。
 */
class WaddleDoo : public Enemy {
public:
    explicit WaddleDoo(QGraphicsItem *player = nullptr, QGraphicsItem *parent = nullptr);

    void update() override;
    QString getEnemyType() const override { return "WaddleDoo"; }
    bool isBeamActive() const { return currentState == STATE_ATTACK; }

protected:
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;

private:
    enum State {
        STATE_WALK,
        STATE_ATTACK,
        STATE_DEAD
    };

    void loadWalkFrames();
    void loadAttackFrames();
    void updateStateMachine();
    void updateAnimation();
    void updateFacingDirection();
    void addBeamShape(QPainterPath &path, int frame) const;

    State currentState = STATE_WALK;
    int currentFrame = 3;
    int stateTimer = 0;
    int cooldownTimer = 0;
    int frameCounter = 0;
    int facingDirection = -1; // -1 = left, +1 = right
    int patrolDirection = -1; // 保留巡邏時的方向，用於攻擊後恢復行走
    bool inPhysicsCheck = false; // 👈 請在 .h 內加上這一行

    static constexpr int WALK_FRAME_COUNT = 5;
    static constexpr int ATTACK_FRAME_START = 3;
    static constexpr int ATTACK_FRAME_END = 12;
    static constexpr int COOLDOWN_DURATION = 60;
    static constexpr int ATTACK_DURATION = 60;

    QPixmap walkFrames[WALK_FRAME_COUNT];
    QPixmap attackFrames[ATTACK_FRAME_END + 1];
    qreal attackScaleX[ATTACK_FRAME_END + 1];
    qreal attackScaleY[ATTACK_FRAME_END + 1];
    int walkFrameIndex = 0;
    qreal currentScaleX = 1.0;
    qreal currentScaleY = 1.0;
    QGraphicsItem *targetPlayer = nullptr;
};

#endif // WADDLEDOO_H
