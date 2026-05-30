#ifndef BOSS_H
#define BOSS_H

#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>

class Kirby;

class Boss : public QGraphicsPixmapItem {
public:
    enum class BossState {
        SmallHop,
        BigJumpToKirby,
        JumpBack,
        VerticalHopPrepare,
        DropBomb,
        Hurt,
        Dead
    };

    Boss(double leftBound, double rightBound, double groundY, Kirby *player);

    void update();
    void takeDamage(int amount);
    bool isDead() const;
    BossState getState() const { return state; }

private:
    void applyPhysics();
    void clampToArena();
    void launchSmallHop();
    void launchBigJumpToKirby();
    void launchJumpBack();
    void launchVerticalHop();
    void handleLanding();
    void setState(BossState newState);
    void updateHealthBar();
    double standingY() const;

    int hp = 3;
    bool dead = false;
    double vx = 0.0;
    double vy = 0.0;
    double gravity = 0.8;
    double leftBound = 0.0;
    double rightBound = 0.0;
    double groundY = 0.0;
    Kirby *player = nullptr;

    BossState state = BossState::SmallHop;
    bool onGround = true;
    int stateTimer = 0;
    int smallHopCounter = 0;
    int moveDirection = 1;

    const int maxHp = 3;
    const int smallHopCountBeforeAttack = 3;
    const double smallHopVx = 3.0;
    const double smallHopVy = -10.0;
    const int smallHopInterval = 25;
    const double bigJumpVx = 7.0;
    const double bigJumpVy = -16.0;
    const double jumpBackVx = 5.5;
    const double jumpBackVy = -13.0;
    const double verticalHopVy = -12.0;

    QGraphicsRectItem *healthBack = nullptr;
    QGraphicsRectItem *healthFill = nullptr;
};

#endif // BOSS_H
