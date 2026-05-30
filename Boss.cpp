#include "Boss.h"
#include "Kirby.h"

#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QtGlobal>

Boss::Boss(double leftBound, double rightBound, double groundY, Kirby *player)
    : leftBound(leftBound),
      rightBound(rightBound),
      groundY(groundY),
      player(player)
{
    QPixmap bossPixmap(":/Project2_Dataset/Image/boss/boss_jump(1).png");
    if (!bossPixmap.isNull()) {
        setPixmap(bossPixmap.scaledToHeight(180, Qt::SmoothTransformation));
    }

    setShapeMode(QGraphicsPixmapItem::MaskShape);
    setZValue(2);
    setPos(this->leftBound, standingY());

    healthBack = new QGraphicsRectItem(this);
    healthBack->setBrush(QBrush(Qt::darkGray));
    healthBack->setPen(QPen(Qt::black));
    healthBack->setZValue(1);

    healthFill = new QGraphicsRectItem(this);
    healthFill->setBrush(QBrush(Qt::red));
    healthFill->setPen(QPen(Qt::NoPen));
    healthFill->setZValue(2);

    updateHealthBar();
}

void Boss::update()
{
    if (dead) return;

    stateTimer++;

    if (state == BossState::SmallHop && onGround && stateTimer >= smallHopInterval) {
        launchSmallHop();
    } else if (state == BossState::BigJumpToKirby && onGround) {
        launchBigJumpToKirby();
    } else if (state == BossState::JumpBack && onGround) {
        launchJumpBack();
    } else if (state == BossState::VerticalHopPrepare && onGround) {
        launchVerticalHop();
    } else if (state == BossState::DropBomb && onGround) {
        // TODO Phase 2: spawn Bomb here.
        setState(BossState::SmallHop);
    }

    applyPhysics();
    updateHealthBar();
}

void Boss::takeDamage(int amount)
{
    if (dead || amount <= 0) return;

    hp -= amount;
    if (hp <= 0) {
        hp = 0;
        dead = true;
        state = BossState::Dead;
        setVisible(false);
    }

    updateHealthBar();
}

bool Boss::isDead() const
{
    return dead;
}

void Boss::applyPhysics()
{
    if (onGround && vy == 0.0) {
        vx = 0.0;
        return;
    }

    vy += gravity;
    setX(x() + vx);
    setY(y() + vy);
    clampToArena();

    if (y() >= standingY()) {
        setY(standingY());
        vy = 0.0;
        onGround = true;
        handleLanding();
    }
}

void Boss::clampToArena()
{
    if (x() < leftBound) {
        setX(leftBound);
        moveDirection = 1;
        if (vx < 0.0) vx = 0.0;
    } else if (x() > rightBound) {
        setX(rightBound);
        moveDirection = -1;
        if (vx > 0.0) vx = 0.0;
    }
}

void Boss::launchSmallHop()
{
    if (x() <= leftBound + 1.0) {
        moveDirection = 1;
    } else if (x() >= rightBound - 1.0) {
        moveDirection = -1;
    }

    vx = smallHopVx * moveDirection;
    vy = smallHopVy;
    onGround = false;
    stateTimer = 0;
}

void Boss::launchBigJumpToKirby()
{
    moveDirection = (player && player->x() < x()) ? -1 : 1;
    vx = bigJumpVx * moveDirection;
    vy = bigJumpVy;
    onGround = false;
    stateTimer = 0;
}

void Boss::launchJumpBack()
{
    moveDirection = (player && player->x() < x()) ? 1 : -1;
    vx = jumpBackVx * moveDirection;
    vy = jumpBackVy;
    onGround = false;
    stateTimer = 0;
}

void Boss::launchVerticalHop()
{
    vx = 0.0;
    vy = verticalHopVy;
    onGround = false;
    stateTimer = 0;
}

void Boss::handleLanding()
{
    vx = 0.0;

    switch (state) {
    case BossState::SmallHop:
        smallHopCounter++;
        if (smallHopCounter >= smallHopCountBeforeAttack) {
            smallHopCounter = 0;
            setState(BossState::BigJumpToKirby);
        } else {
            stateTimer = 0;
        }
        break;
    case BossState::BigJumpToKirby:
        setState(BossState::JumpBack);
        break;
    case BossState::JumpBack:
        setState(BossState::VerticalHopPrepare);
        break;
    case BossState::VerticalHopPrepare:
        setState(BossState::DropBomb);
        break;
    default:
        break;
    }
}

void Boss::setState(BossState newState)
{
    state = newState;
    stateTimer = 0;
}

void Boss::updateHealthBar()
{
    if (!healthBack || !healthFill) return;

    const qreal barWidth = 90.0;
    const qreal barHeight = 10.0;
    const qreal barX = (boundingRect().width() - barWidth) / 2.0;
    const qreal barY = -18.0;
    const qreal hpRatio = maxHp > 0 ? qBound(0.0, static_cast<double>(hp) / maxHp, 1.0) : 0.0;

    healthBack->setRect(barX, barY, barWidth, barHeight);
    healthFill->setRect(barX + 1.0, barY + 1.0, (barWidth - 2.0) * hpRatio, barHeight - 2.0);
}

double Boss::standingY() const
{
    return groundY - boundingRect().height();
}
