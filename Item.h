#ifndef ITEM_H
#define ITEM_H

// ==========================================
// 1. Qt 內建函式庫引入
// ==========================================
#include <QGraphicsPixmapItem>
#include <QObject>

// ==========================================
// 2. 前向宣告 (Forward Declaration)
//    避免循環引用：Item.h 不 #include Kirby.h，
//    只告訴編譯器「有個 Kirby 類別存在」。
//    實際的 include 放在 Item.cpp 中。
// ==========================================
class Kirby;

/**
 * @brief Item 所有道具的抽象基底類別 (Base Class)
 * @details 繼承自 QObject 與 QGraphicsPixmapItem，
 * 讓道具可以擺放在場景中且支援 Qt 訊號槽機制。
 * 每個子類別必須實作 onConsumed() 以定義被拾取後的效果。
 */
class Item : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    explicit Item(QGraphicsItem *parent = nullptr);
    virtual ~Item() = default;

    /**
     * @brief 道具被卡比接觸後觸發的效果 (純虛擬函式)
     * @param kirby 接觸此道具的卡比指標
     * @note 子類別必須在此函式中呼叫對應的 Kirby 方法，
     * 然後呼叫 scene()->removeItem(this) 將自己從場景移除。
     */
    virtual void onConsumed(Kirby *kirby) = 0;

    /**
     * @brief 取得道具是否已被消費 (防止重複觸發)
     */
    bool isConsumed() const { return consumed; }

protected:
    bool consumed = false; ///< 標記道具是否已被消費，防止同一幀觸發多次
};


// ==========================================
// MaximTomato — 恢復滿血
// ==========================================
/**
 * @brief MaximTomato 道具類別
 * @details 卡比接觸後呼叫 kirby->restoreFullHP()，
 * 並立即從場景移除自身。
 */
class MaximTomato : public Item {
    Q_OBJECT

public:
    explicit MaximTomato(QGraphicsItem *parent = nullptr);

    /**
     * @brief 觸發滿血回復效果
     * @param kirby 接觸此道具的卡比
     */
    void onConsumed(Kirby *kirby) override;
};


// ==========================================
// OneUp — 增加一條命
// ==========================================
/**
 * @brief OneUp 道具類別
 * @details 卡比接觸後呼叫 kirby->addLife(1)，
 * 並立即從場景移除自身。
 */
class OneUp : public Item {
    Q_OBJECT

public:
    explicit OneUp(QGraphicsItem *parent = nullptr);

    /**
     * @brief 觸發增命效果
     * @param kirby 接觸此道具的卡比
     */
    void onConsumed(Kirby *kirby) override;
};

#endif // ITEM_H
