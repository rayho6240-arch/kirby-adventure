#include "WaddleDoo.h"
#include <QPainter>
#include <QDebug>
#include <cmath>

// =========================================================
// 1. 初始化與建構子
// =========================================================

WaddleDoo::WaddleDoo(QGraphicsItem *player, QGraphicsItem *parent)
    : Enemy(parent), targetPlayer(player) {
    enemyType = "WaddleDoo";
    vx = -2.0;
    facingDirection = -1;
    currentState = STATE_WALK;
    currentFrame = ATTACK_FRAME_START;
    stateTimer = 0;
    cooldownTimer = 0;
    frameCounter = 0;
    inPhysicsCheck = false; // 👈 記得在建構子內初始化為 false

    loadWalkFrames();
    loadAttackFrames();
    walkFrameIndex = 0;
    setPixmap(walkFrames[walkFrameIndex]);
}

// =========================================================
// 2. 每幀更新邏輯
// =========================================================

void WaddleDoo::update() {
    if (isDead) {
        currentState = STATE_DEAD;
        return;
    }
    
    updateStateMachine();
    updateAnimation();
    
    // ⭕ 關鍵修正：在執行地形物理排斥時，開啟「物理檢測中」旗標
    inPhysicsCheck = true; 
    handlePhysics(100, 100); // 這時候物理引擎看不見光束，絕對不會再被彈飛！
    inPhysicsCheck = false;  // 檢測完馬上關閉，確保後續卡比走過來時會正常被光束揍
    
    // 同步面向方向與巡邏方向（當碰到方塊時轉向）
    if (currentState == STATE_WALK && vx != 0) {
        patrolDirection = (vx > 0) ? 1 : -1;
        facingDirection = patrolDirection;
    }
}

QRectF WaddleDoo::boundingRect() const {
    // ⭕ 關鍵修正：如果物理引擎在問邊界，直接給它標準的 100x100，不讓它被光束干擾
    if (inPhysicsCheck) {
        return QRectF(0, 0, 100, 100);
    }
    // 1. 計算圖片動態縮放（以 50, 100 為軸心）後的實際視覺範圍
    qreal visualX = 50.0 - 50.0 * currentScaleX;
    qreal visualY = 100.0 - 100.0 * currentScaleY;
    qreal visualW = 100.0 * currentScaleX;
    qreal visualH = 100.0 * currentScaleY;
    QRectF visualRect(visualX, visualY, visualW, visualH);

    // 2. 取「視覺範圍」與「物理碰撞箱 `shape()`」的聯集
    // 這樣不論是拉伸得很高、還是光束伸得很遠，通通都會被完整包覆，絕對不破圖！
    return visualRect.united(shape().boundingRect());
}

// =========================================================
// 3. 狀態機與攻擊流程
// =========================================================

void WaddleDoo::updateStateMachine() {
    if (currentState == STATE_DEAD) {
        return;
    }

    if (currentState == STATE_ATTACK) {
        stateTimer += 1;
        if (stateTimer % 6 == 0 && currentFrame < ATTACK_FRAME_END) {
            currentFrame += 1;
        }

        if (stateTimer >= ATTACK_DURATION) {
            currentState = STATE_WALK;
            stateTimer = 0;
            cooldownTimer = COOLDOWN_DURATION;
            currentFrame = ATTACK_FRAME_START;
            vx = patrolDirection * 2.0;
            facingDirection = patrolDirection;
        } else {
            vx = 0;
        }
        return;
    }

    if (cooldownTimer > 0) {
        cooldownTimer -= 1;
        vx = 0;
        return;
    }

    // 行走狀態：若玩家靠近，發動攻擊
    if (targetPlayer) {
        qreal dx = targetPlayer->x() - x();
        qreal dy = targetPlayer->y() - y();
        qreal distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= 280) {
            // 只在攻擊範圍內時，面向玩家
            updateFacingDirection();
            currentState = STATE_ATTACK;
            currentFrame = ATTACK_FRAME_START;
            stateTimer = 0;
            vx = 0;
            return;
        }
    }

    currentState = STATE_WALK;
    if (currentState == STATE_WALK) {
        vx = patrolDirection * 2.0;
    }
}

void WaddleDoo::updateFacingDirection() {
    if (!targetPlayer) return;
    qreal dx = targetPlayer->x() - x();
    if (dx == 0) return;
    facingDirection = dx > 0 ? 1 : -1;
}

// =========================================================
// 4. 動畫與繪製
// =========================================================

void WaddleDoo::updateAnimation() {
    frameCounter += 1;

    // ⭕ 關鍵修正：因為接下來的邏輯會改變縮放比例，進而改變 boundingRect 的大小，
    // 必須先通知 Qt 刷新幾何緩存，徹底解決破圖與殘影問題！
    prepareGeometryChange();

    if (currentState == STATE_ATTACK) {
        setPixmap(attackFrames[currentFrame]);
        currentScaleX = attackScaleX[currentFrame];
        currentScaleY = attackScaleY[currentFrame];
    } else {
        if (WALK_FRAME_COUNT > 0) {
            walkFrameIndex = (frameCounter / 10) % WALK_FRAME_COUNT;
            setPixmap(walkFrames[walkFrameIndex]);
        }
        currentScaleX = 1.0;
        currentScaleY = 1.0;
    }

    QTransform transform;
    qreal horizontalFlip = (facingDirection == -1) ? -1.0 : 1.0;

    // 外層 Transform 只負責左右翻轉
    transform.translate(50, 100);
    transform.scale(horizontalFlip, 1.0); 
    transform.translate(-50, -100);
    setTransform(transform);
}
void WaddleDoo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();

    // ⭕ 視覺形變：在這裡套用你在 loadAttackFrames 寫好的拉伸比例
    painter->translate(50, 100); 
    painter->scale(currentScaleX, currentScaleY);
    painter->translate(-50, -100);

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    // 注意：不用再判斷面向左右翻轉了，因為外層的 setTransform 已經翻轉過了
    painter->drawPixmap(0, 0, pixmap());
    
    painter->restore();
}
// =========================================================
// 5. 動態碰撞形狀：身體 + 光束
// =========================================================

QPainterPath WaddleDoo::shape() const {
    QPainterPath path;
    if (currentState == STATE_DEAD) {
        return path;
    }

    // 身體底部碰撞：固定在左下角（100x100 座標系）
    path.addEllipse(QRectF(10, 40, 55, 55));

    // ⭕ 關鍵修正：只有在「非地形物理檢測」時，才把光束加進碰撞箱
    if (currentState == STATE_ATTACK && !inPhysicsCheck) {
        addBeamShape(path, currentFrame);
    }
    return path;
}

void WaddleDoo::addBeamShape(QPainterPath &path, int frame) const {
    switch (frame) {
    case 3:
        path.addRect(QRectF(81, 14, 25, 17));
        path.addRect(QRectF(104, 31, 25, 14));
        break;
    case 4:
        path.addRect(QRectF(76, 20, 31, 17));
        path.addRect(QRectF(101, 36, 25, 14));
        break;
    case 5:
        path.addRect(QRectF(70, 25, 36, 17));
        path.addRect(QRectF(98, 42, 25, 14));
        break;
    case 6:
        path.addRect(QRectF(64, 34, 42, 17));
        path.addRect(QRectF(106, 48, 22, 14));
        break;
    case 7:
        path.addRect(QRectF(62, 42, 48, 17));
        path.addRect(QRectF(112, 53, 20, 14));
        break;
    case 8:
        path.addRect(QRectF(59, 50, 53, 17));
        path.addRect(QRectF(115, 62, 17, 14));
        break;
    case 9:
        path.addRect(QRectF(56, 59, 59, 17));
        path.addRect(QRectF(118, 70, 17, 14));
        break;
    case 10:
        path.addRect(QRectF(53, 73, 64, 17));
        path.addRect(QRectF(118, 84, 20, 14));
        break;
    case 11:
        path.addRect(QRectF(50, 78, 70, 17));
        path.addRect(QRectF(120, 90, 17, 14));
        break;
    case 12:
        path.addRect(QRectF(48, 81, 76, 17));
        path.addRect(QRectF(123, 92, 14, 14));
        break;
    default:
        break;
    }
}

// =========================================================
// 6. 圖片載入
// =========================================================

void WaddleDoo::loadWalkFrames() {
    for (int frame = 0; frame < WALK_FRAME_COUNT; ++frame) {
        QString path = QString(":/Project2_Dataset/Image/Waddle Doo/walk_%1.png").arg(frame + 1);
        QPixmap pix(path);
        if (pix.isNull()) {
            qDebug() << "WaddleDoo: 無法載入" << path;
            pix = QPixmap(100, 100);
            pix.fill(Qt::transparent);
        }
        else {
            qDebug() << "WaddleDoo: 成功載入" << path;
        }
        walkFrames[frame] = pix.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
}

void WaddleDoo::loadAttackFrames() {
    static const qreal frameScaleX[ATTACK_FRAME_END + 1] = {
        1.0, // placeholder for index 0
        1.0, // placeholder for index 1
        1.0, // placeholder for index 2
        1.0, // frame 3: high beam start
        1.222,
        1.444,
        1.667,
        1.889,
        2.111,
        2.333,
        2.556,
        2.778,
        3.0  // frame 12: widest reach
    };
    static const qreal frameScaleY[ATTACK_FRAME_END + 1] = {
        1.0, // placeholder for index 0
        1.0, // placeholder for index 1
        1.0, // placeholder for index 2
        3.0, // frame 3: starting tallest
        2.778,
        2.556,
        2.333,
        2.111,
        1.889,
        1.667,
        1.444,
        1.222,
        1.0  // frame 12: normal height
    };

    for (int frame = ATTACK_FRAME_START; frame <= ATTACK_FRAME_END; ++frame) {
        QString path = QString(":/Project2_Dataset/Image/Waddle Doo/attack_%1.png").arg(frame);
        QPixmap pix(path);
        if (pix.isNull()) {
            qDebug() << "WaddleDoo: 無法載入" << path;
            pix = QPixmap(100, 100);
            pix.fill(Qt::transparent);
        } else {
            qDebug() << "WaddleDoo: 成功載入" << path;
        }
        attackFrames[frame] = pix.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        attackScaleX[frame] = frameScaleX[frame];
        attackScaleY[frame] = frameScaleY[frame];
    }
}
