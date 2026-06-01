#include "Item.h"
#include "Block.h"
#include "Slope.h"
#include "Kirby.h"      // 在 .cpp 中才真正 include，避免循環引用

#include <QGraphicsScene>
#include <QList>

// =========================================================
// Item 基底類別
// =========================================================

Item::Item(QGraphicsItem *parent): QObject(nullptr), QGraphicsPixmapItem(parent){
    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
}

void Item::update()
{
    if (consumed || !scene()) return;

    const qreal oldY = y();
    const qreal itemHeight = boundingRect().height();
    vy += gravity;
    setY(y() + vy);
    isOnGround = false;

    QList<QGraphicsItem *> itemsY = scene()->collidingItems(this, Qt::IntersectsItemShape);
    for (QGraphicsItem *sceneItem : itemsY) {
        Slope *slope = dynamic_cast<Slope *>(sceneItem);
        if (slope) {
            const qreal footCenterX = sceneBoundingRect().center().x();
            const qreal surfaceY = slope->getSurfaceY(footCenterX);
            const qreal previousBottom = oldY + itemHeight;
            const qreal currentBottom = y() + itemHeight;

            if (vy >= 0 && previousBottom <= surfaceY + 30 && currentBottom >= surfaceY - 8) {
                setY(surfaceY - itemHeight);
                vy = 0;
                isOnGround = true;
                return;
            }

            continue;
        }

        Block *block = qgraphicsitem_cast<Block *>(sceneItem);
        if (block && vy >= 0 && oldY + itemHeight <= block->y() + 30) {
            setY(block->y() - itemHeight);
            vy = 0;
            isOnGround = true;
            return;
        }
    }
}

// =========================================================
// MaximTomato — 恢復滿血
// =========================================================

MaximTomato::MaximTomato(QGraphicsItem *parent): Item(parent){
    // 使用資源檔中已存在的圖片（注意原始檔名含空格）
    QPixmap pix(":/Project2_Dataset/Image/item/Maxim Tomato.png");
    if (!pix.isNull()) {
        pix = pix.scaledToHeight(60, Qt::SmoothTransformation);
        setPixmap(pix);
    }
    setZValue(15); // 確保在背景與平台前方
}

void MaximTomato::onConsumed(Kirby *kirby) {
    if (consumed) return;   // 防止同一幀重複觸發
    consumed = true;

    kirby->restoreFullHP(); // 呼叫卡比的滿血回復

    // 先從場景移除，再透過 Qt 事件迴圈安全銷毀
    if (scene()) scene()->removeItem(this);
    deleteLater();
}

// =========================================================
// OneUp — 增加一條命
// =========================================================

OneUp::OneUp(QGraphicsItem *parent): Item(parent){
    QPixmap pix(":/Project2_Dataset/Image/item/1UP.png");
    if (!pix.isNull()) {
        pix = pix.scaledToHeight(60, Qt::SmoothTransformation);
        setPixmap(pix);
    }
    setZValue(15);
}

void OneUp::onConsumed(Kirby *kirby) {
    if (consumed) return;
    consumed = true;

    kirby->addLife(1);      // 呼叫卡比的增命函式

    if (scene()) scene()->removeItem(this);
    deleteLater();
}
