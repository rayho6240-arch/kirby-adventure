#include "Item.h"
#include "Kirby.h"      // 在 .cpp 中才真正 include，避免循環引用

#include <QGraphicsScene>

// =========================================================
// Item 基底類別
// =========================================================

Item::Item(QGraphicsItem *parent): QObject(nullptr), QGraphicsPixmapItem(parent){
    //empty
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
    setZValue(5); // 確保在背景之上
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
    setZValue(5);
}

void OneUp::onConsumed(Kirby *kirby) {
    if (consumed) return;
    consumed = true;

    kirby->addLife(1);      // 呼叫卡比的增命函式

    if (scene()) scene()->removeItem(this);
    deleteLater();
}
