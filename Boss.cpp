#include "Boss.h"
#include "Kirby.h"
#include "Slope.h"

#include <QBrush>
#include <QGraphicsScene>
#include <QPen>
#include <QPixmap>
#include <QString>
#include <QtGlobal>
#include <QDebug>
#include <cmath>

Boss::Boss(double leftBound, double rightBound, double groundY, Kirby *player)
    : leftBound(leftBound),
      rightBound(rightBound),
      groundY(groundY),
      player(player)
{
    loadAnimationPixmaps();
    if (!fallbackPixmap.isNull()) {
        setPixmap(fallbackPixmap);
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
        // Bomb is requested at the VerticalHopPrepare apex; this state closes the cycle.
        setState(BossState::SmallHop);
    }

    applyPhysics();
    updateSprite();
    updateHealthBar();
}

void Boss::takeDamage(int amount)
{
    if (dead || amount <= 0) return;

    hp -= amount;
    qDebug() << "Boss HP =" << hp;
    if (hp <= 0) {
        hp = 0;
        dead = true;
        state = BossState::Dead;
        setVisible(false);
        qDebug() << "Boss defeated";
    }

    updateHealthBar();
}

bool Boss::isDead() const
{
    return dead;
}

bool Boss::consumeBombSpawnRequest(QPointF &pos, double &vx, double &vy)
{
    if (!bombSpawnRequested) return false;

    pos = requestedBombPos;
    vx = requestedBombVx;
    vy = requestedBombVy;
    bombSpawnRequested = false;
    return true;
}

void Boss::applyPhysics()
{
    if (onGround && vy == 0.0) {
        vx = 0.0;
        return;
    }

    const double previousVy = vy;
    vy += gravity;
    if (state == BossState::VerticalHopPrepare && !bombDroppedThisCycle && previousVy < 0.0 && vy >= 0.0) {
        requestBombSpawn();
        bombDroppedThisCycle = true;
    }

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
    if (newState == BossState::VerticalHopPrepare) {
        bombDroppedThisCycle = false;
    }
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

void Boss::requestBombSpawn()
{
    const QPointF bombPos = sceneBoundingRect().center();
    QPointF targetPos = player ? player->sceneBoundingRect().center() : QPointF(bombPos.x() + moveDirection, bombPos.y());
    targetPos.setY(targetPos.y() - aimYOffset);

    double dx = targetPos.x() - bombPos.x();
    double dy = targetPos.y() - bombPos.y();
    double length = std::sqrt(dx * dx + dy * dy);

    if (length < 1.0) {
        dx = player && player->x() < x() ? -1.0 : 1.0;
        dy = -0.8;
        length = std::sqrt(dx * dx + dy * dy);
    }

    requestedBombPos = bombPos;
    requestedBombVx = bombLaunchSpeed * dx / length;
    requestedBombVy = bombLaunchSpeed * dy / length;
    bombSpawnRequested = true;

    qDebug() << "Boss requested bomb spawn";
}

void Boss::loadAnimationPixmaps()
{
    auto loadBossPixmap = [](const QString &path) {
        QPixmap pixmap(path);
        if (!pixmap.isNull()) {
            return pixmap.scaledToHeight(180, Qt::SmoothTransformation);
        }
        return pixmap;
    };

    fallbackPixmap = loadBossPixmap(":/Project2_Dataset/Image/boss/boss_jump(1).png");

    jumpFaceL1L = loadBossPixmap(":/Project2_Dataset/Image/boss/faceL/jump_faceL_(1)_L.png");
    jumpFaceL1R = loadBossPixmap(":/Project2_Dataset/Image/boss/faceL/jump_faceL_(1)_R.png");
    jumpFaceL2L = loadBossPixmap(":/Project2_Dataset/Image/boss/faceL/jump_faceL_(2)_L.png");
    jumpFaceL2R = loadBossPixmap(":/Project2_Dataset/Image/boss/faceL/jump_faceL_(2)_R.png");

    jumpFaceR1L = loadBossPixmap(":/Project2_Dataset/Image/boss/faceR/jump_faceR_(1)_L.png");
    jumpFaceR1R = loadBossPixmap(":/Project2_Dataset/Image/boss/faceR/jump_faceR_(1)_R.png");
    jumpFaceR2L = loadBossPixmap(":/Project2_Dataset/Image/boss/faceR/jump_faceR_(2)_L.png");
    jumpFaceR2R = loadBossPixmap(":/Project2_Dataset/Image/boss/faceR/jump_faceR_(2)_R.png");

    drop1L = loadBossPixmap(":/Project2_Dataset/Image/boss/drop_(1)_L.png");
    drop1R = loadBossPixmap(":/Project2_Dataset/Image/boss/drop_(1)_R.png");
    drop2L = loadBossPixmap(":/Project2_Dataset/Image/boss/drop_(2)_L.png");
    drop2R = loadBossPixmap(":/Project2_Dataset/Image/boss/drop_(2)_R.png");
}

void Boss::updateSprite()
{
    if (dead) return;

    const qreal bossCenterX = sceneBoundingRect().center().x();
    const qreal playerCenterX = player ? player->sceneBoundingRect().center().x() : bossCenterX + moveDirection;
    const bool faceLeft = playerCenterX < bossCenterX;

    if (state == BossState::VerticalHopPrepare && !onGround) {
        const double apexVyThreshold = 2.0;
        const bool useDrop1 = vy < 0.0 && std::abs(vy) > apexVyThreshold;
        const QPixmap &dropPixmap = useDrop1
                                        ? (faceLeft ? drop1L : drop1R)
                                        : (faceLeft ? drop2L : drop2R);
        setBossPixmap(dropPixmap);
        return;
    }

    if (!onGround) {
        const bool movingRight = vx >= 0.0;
        const int phase = (stateTimer / 5) % 4;
        int frame = 1;
        bool frameLeft = movingRight;

        if (movingRight) {
            frame = (phase == 0 || phase == 3) ? 1 : 2;
            frameLeft = (phase == 0 || phase == 1);
        } else {
            frame = (phase == 0 || phase == 3) ? 1 : 2;
            frameLeft = (phase == 2 || phase == 3);
        }

        setBossPixmap(jumpPixmap(faceLeft, frame, frameLeft));
        return;
    }

    setBossPixmap(jumpPixmap(faceLeft, 1, moveDirection >= 0));
}

void Boss::setBossPixmap(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        if (!fallbackPixmap.isNull()) {
            setBossPixmap(fallbackPixmap);
        }
        return;
    }

    const qreal oldBottom = sceneBoundingRect().bottom();
    setPixmap(pixmap);

    if (oldBottom != 0.0) {
        const qreal newBottom = sceneBoundingRect().bottom();
        setY(y() + (oldBottom - newBottom));
    }
}

const QPixmap &Boss::jumpPixmap(bool faceLeft, int frame, bool frameLeft) const
{
    if (faceLeft) {
        if (frame == 1) return frameLeft ? jumpFaceL1L : jumpFaceL1R;
        return frameLeft ? jumpFaceL2L : jumpFaceL2R;
    }

    if (frame == 1) return frameLeft ? jumpFaceR1L : jumpFaceR1R;
    return frameLeft ? jumpFaceR2L : jumpFaceR2R;
}

double Boss::standingY() const
{
    qreal surfaceY = groundY;

    if (scene()) {
        const qreal footCenterX = sceneBoundingRect().center().x();
        for (QGraphicsItem *item : scene()->items()) {
            Slope *slope = dynamic_cast<Slope *>(item);
            if (!slope) continue;

            const QRectF slopeRect = slope->sceneBoundingRect();
            if (footCenterX < slopeRect.left() || footCenterX > slopeRect.right()) {
                continue;
            }

            const qreal candidateY = slope->getSurfaceY(footCenterX);
            if (candidateY < surfaceY) {
                surfaceY = candidateY;
            }
        }
    }

    return surfaceY - boundingRect().height();
}
