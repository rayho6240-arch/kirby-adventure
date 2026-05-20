#include "HotHead.h"
#include "Kirby.h"
#include <QPixmap>
#include <QDebug>
#include <cmath>
#include <QGraphicsScene>
#include <QList>

// =========================================================
// 1. 初始化與建構子 (Initialization)
// =========================================================

HotHead::HotHead(QGraphicsItem *player, QGraphicsItem *parent)
    : Enemy(parent), targetPlayer(player) {

    enemyType = "HotHead";

    runLeft.load(":/Project2_Dataset/Image/Hot Head/Hot_head_run_L(2).png");
    runLeft = runLeft.scaled(60, 60, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    runRight.load(":/Project2_Dataset/Image/Hot Head/Hot_head_run_R(2).png");
    runRight = runRight.scaled(60, 60, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    attackLeft.load(":/Project2_Dataset/Image/Hot Head/Hot_head_attack_L.png");
    attackLeft = attackLeft.scaled(60, 60, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    attackRight.load(":/Project2_Dataset/Image/Hot Head/Hot_head_attack_R.png");
    attackRight = attackRight.scaled(60, 60, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    fire1.load(":/Project2_Dataset/Image/Hot Head/Hot_head_fire(1).png");
    fire1 = fire1.scaled(60, 60, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    fire2Left.load(":/Project2_Dataset/Image/Hot Head/Hot_head_fire(2)_L.png");
    fire2Left = fire2Left.scaled(60, 60, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    fire2Right.load(":/Project2_Dataset/Image/Hot Head/Hot_head_fire(2)_R.png");
    fire2Right = fire2Right.scaled(60, 60, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    fire3Left.load(":/Project2_Dataset/Image/Hot Head/Hot_head_fire(3)_L.png");
    fire3Left = fire3Left.scaled(60, 60, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    fire3Right.load(":/Project2_Dataset/Image/Hot Head/Hot_head_fire(3)_R.png");
    fire3Right = fire3Right.scaled(60, 60, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    setPixmap(runLeft);
    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);

    fireEffect = new QGraphicsPixmapItem(this);
    fireEffect->setVisible(false);
    fireEffect->setZValue(1);

    currentState = PATROL;
    stateTimer = 0.0f;
    facingDirection = -1;
    vx = -PATROL_SPEED;
}

// =========================================================
// 2. 核心更新邏輯 (Core Update)
// =========================================================

void HotHead::update() {
    if (isDead) return;

    switch (currentState) {
    case PATROL:
        checkPlayerDistance();
        handlePhysics(60, 60);
        updateSprite();
        break;

    case CHASE:
        updateFacingDirection();
        stateTimer += 1.0f;
        if (stateTimer >= CHASE_DURATION) {
            currentState = CHARGE;
            stateTimer = 0.0f;
        }
        updateSprite();
        break;

    case CHARGE:
        updateFacingDirection();
        stateTimer += 1.0f;
        if (stateTimer >= CHARGE_DURATION) {
            currentState = FIRE;
            stateTimer = 0.0f;
            frameCounter = 0;
            vx = 0;
        }
        updateSprite();
        break;

    case FIRE:
        stateTimer += 1.0f;
        updateSprite();
        applyFireDamage();
        if (stateTimer >= FIRE_DURATION) {
            currentState = COOLDOWN;
            stateTimer = 0.0f;
            if (fireEffect) fireEffect->setVisible(false);
        }
        break;

    case COOLDOWN:
        updateFacingDirection();
        stateTimer += 1.0f;
        if (stateTimer >= COOLDOWN_DURATION) {
            currentState = PATROL;
            stateTimer = 0.0f;
            vx = facingDirection * PATROL_SPEED;
        }
        updateSprite();
        break;
    }
}

void HotHead::draw() {
    updateSprite();
}

// =========================================================
// 3. 玩家距離偵測 (Player Distance Detection)
// =========================================================

void HotHead::checkPlayerDistance() {
    if (!targetPlayer) return;

    qreal dx = targetPlayer->x() - x();
    qreal dy = targetPlayer->y() - y();
    qreal distance = std::sqrt(dx * dx + dy * dy);

    if (distance < DETECT_RANGE) {
        currentState = CHASE;
        stateTimer = 0.0f;
        vx = 0;
        facingDirection = dx >= 0 ? 1 : -1;
    }
}

void HotHead::updateFacingDirection() {
    if (!targetPlayer) return;
    qreal dx = targetPlayer->x() - x();
    if (dx == 0) return;
    facingDirection = dx > 0 ? 1 : -1;
}

// =========================================================
// 4. 動畫更新 (Animation Update)
// =========================================================

void HotHead::updateSprite() {
    frameCounter += 1;

    switch (currentState) {
    case PATROL:
        setPixmap(facingDirection == 1 ? runRight : runLeft);
        break;

    case CHASE:
        setPixmap(facingDirection == 1 ? runRight : runLeft);
        break;

    case CHARGE:
        setPixmap(facingDirection == 1 ? attackRight : attackLeft);
        if (fireEffect) fireEffect->setVisible(false);
        break;

    case FIRE: {
        setPixmap(facingDirection == 1 ? attackRight : attackLeft);
        updateFireEffect();
        break;
    }

    case COOLDOWN:
        setPixmap(facingDirection == 1 ? runRight : runLeft);
        break;
    }
}

void HotHead::updateFireEffect() {
    if (!fireEffect) return;

    int cycle = static_cast<int>(std::floor(stateTimer / 20.0f)) % 3;
    if (cycle == 0) {
        fireEffect->setPixmap(fire1);
    } else if (cycle == 1) {
        fireEffect->setPixmap(facingDirection == 1 ? fire2Right : fire2Left);
    } else {
        fireEffect->setPixmap(facingDirection == 1 ? fire3Right : fire3Left);
    }

    fireEffect->setVisible(true);
    const qreal offsetX = facingDirection == 1 ? 40 : -40;
    const qreal offsetY = 0;
    fireEffect->setPos(offsetX, offsetY);
}

void HotHead::applyFireDamage() {
    if (!targetPlayer || !fireEffect) return;

    QList<QGraphicsItem*> colliding = scene()->collidingItems(fireEffect);
    for (QGraphicsItem *item : colliding) {
        Kirby *kirby = dynamic_cast<Kirby*>(item);
        if (kirby) {
            if (frameCounter % 10 == 0) {
                kirby->takeDamage(1);
            }
        }
    }
}
