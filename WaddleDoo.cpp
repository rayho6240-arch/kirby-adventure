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
/*  實作被吸邏輯：一旦被吸，就強制切換到 BEING_INHALED 狀態，並且完全不受其他狀態影響
    此部分待修改
    if (isInhaled) {
        if (currentState != BEING_INHALED) {
            currentState = BEING_INHALED;
        }
    }

    if (currentState == BEING_INHALED) {
        if (targetPlayer != nullptr) {
            qreal dist = qAbs(targetPlayer->x() - this->x());
            if (dist < 20) {
                setIsDead(true);
                setVisible(false);
                return;
            }
        }
        handlePhysics(100, 100);
        updateSprite();
        return;
    }
*/    
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
    prepareGeometryChange();

    if (currentState == STATE_ATTACK) {
        // 👉 核心邏輯：利用這兩張乾淨的身體，實現眼睛顏色閃爍！
        // 奇數幀用 charge，偶數幀用 walk
        if (currentFrame % 2 != 0) {
            setPixmap(attackWaddleDooChargePixmap);
        } else {
            setPixmap(attackWaddleDooWalkPixmap);
        }
    } else {
            if (WALK_FRAME_COUNT > 0) {
                walkFrameIndex = (frameCounter / 10) % WALK_FRAME_COUNT;
                setPixmap(walkFrames[walkFrameIndex]);
            }
    }

    // ⭕ 本體比例徹底回歸 1.0 原始比例，不再拉扁
    currentScaleX = 1.0;
    currentScaleY = 1.0;

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
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    // 1. 繪製 Waddle Doo 本體 (此時身體是完美的 1:1，不再被拉長)
    painter->drawPixmap(0, 0, pixmap());
    
    // 2. ⭕ 核心進階：如果正在攻擊，程式自動在對應位置畫出 attack_star
    if (currentState == STATE_ATTACK) {
        // 抓取座標黑盒子裡的矩形資料
        for (const QRectF &rect : getBeamRects(currentFrame)) {
            // 把單張 attack_star 精準繪製到指定的矩形區域內
            painter->drawPixmap(rect, attackStarPixmap, attackStarPixmap.rect());
        }
    }
    
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

    // 1. 身體底部碰撞
    path.addEllipse(QRectF(10, 40, 55, 55));

    // 2. ⭕ 完美整合：直接呼叫 getBeamRects，把星星方塊加進碰撞路徑中
    if (currentState == STATE_ATTACK && !inPhysicsCheck) {
        for (const QRectF &rect : getBeamRects(currentFrame)) {
            path.addRect(rect);
        }
    }
    
    return path;
}

QList<QRectF> WaddleDoo::getBeamRects(int frame) const {
    QList<QRectF> rects;
    
    // 每一顆星星的大小固定為 24x24 像素
    qreal w = 24;
    qreal h = 24;

    switch(frame) {
        case 3: // 第 3 幀：2顆星星，甩在右上方
            rects.append(QRectF(75, 20, w, h));
            rects.append(QRectF(65, -20, w, h));
            break;
            
        case 4: // 第 4 幀：3顆星星，垂直弧線
            rects.append(QRectF(80, 25, w, h));
            rects.append(QRectF(85, -15, w, h));
            rects.append(QRectF(70, -50, w, h));
            break;
            
        case 5:
            rects.append(QRectF(85, 30, w, h));
            rects.append(QRectF(100, -5, w, h));
            rects.append(QRectF(90, -40, w, h));
            break;
            
        case 6:
            rects.append(QRectF(90, 35, w, h));
            rects.append(QRectF(115, 5, w, h));
            rects.append(QRectF(110, -25, w, h));
            break;
            
        case 7: // 鞭子逐漸往前延伸
            rects.append(QRectF(95, 40, w, h));
            rects.append(QRectF(125, 20, w, h));
            rects.append(QRectF(130, -5, w, h));
            break;
            
        case 8:
            rects.append(QRectF(100, 45, w, h));
            rects.append(QRectF(135, 35, w, h));
            rects.append(QRectF(145, 15, w, h));
            break;
            
        case 9:
            rects.append(QRectF(100, 50, w, h));
            rects.append(QRectF(140, 50, w, h));
            rects.append(QRectF(155, 30, w, h));
            break;
            
        case 10:
            rects.append(QRectF(95, 55, w, h));
            rects.append(QRectF(135, 65, w, h));
            rects.append(QRectF(160, 50, w, h));
            break;
            
        case 11: // 鞭子甩到下方
            rects.append(QRectF(90, 60, w, h));
            rects.append(QRectF(125, 75, w, h));
            rects.append(QRectF(155, 70, w, h));
            break;
            
        case 12: // 第 12 幀：最後的收尾
            rects.append(QRectF(85, 65, w, h));
            rects.append(QRectF(115, 80, w, h));
            rects.append(QRectF(145, 80, w, h));
            break;
            
        default:
            break;
    }
    
    return rects;
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
    prepareGeometryChange(); // 通知幾何改變

    // 1. 載入偶數幀身體 (image_11)
    attackWaddleDooWalkPixmap = QPixmap(":/Project2_Dataset/Image/Waddle Doo/attack_WaddleDoo.1.png");
    if (!attackWaddleDooWalkPixmap.isNull()) {
        attackWaddleDooWalkPixmap = attackWaddleDooWalkPixmap.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // 2. 載入奇數幀身體 (image_12)
    attackWaddleDooChargePixmap = QPixmap(":/Project2_Dataset/Image/Waddle Doo/attack_WaddleDoo.2.png");
    if (!attackWaddleDooChargePixmap.isNull()) {
        attackWaddleDooChargePixmap = attackWaddleDooChargePixmap.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // 3. 載入星星特效 (image_10)
    attackStarPixmap = QPixmap(":/Project2_Dataset/Image/Waddle Doo/attack_star.png");
    if (attackStarPixmap.isNull()) {
        // 防閃退備用方案
        attackStarPixmap = QPixmap(20, 20);
        attackStarPixmap.fill(Qt::yellow);
    }
}
