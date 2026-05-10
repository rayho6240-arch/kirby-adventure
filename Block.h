#ifndef BLOCK_H
#define BLOCK_H

// ==========================================
// 1. Qt 內建函式庫引入
// ==========================================
#include <QGraphicsRectItem>

/**
 * @brief Block 地形與障礙物的基底類別
 * @details 繼承自 QGraphicsRectItem。通常作為遊戲中「隱形碰撞箱」使用，
 * 例如隱形地板、空氣牆等。未來也可以延伸出可破壞的星之方塊 (StarBlock)。
 */
class Block : public QGraphicsRectItem {
public:
    // ==========================================
    // Qt 轉型辨識系統 (RTTI / Type System)
    // ==========================================
    //Kirby.cpp中x軸碰撞很重要的機制在這裡，這些block 有他們自己的編號，他們都屬於<block>type 這樣其他小方塊如:星星就不會觸發碰撞
    /**
     * @brief 定義專屬的型別 ID
     * @note 這是給 qgraphicsitem_cast 辨識用的身分證號碼。
     * UserType 是 Qt 預留給開發者自定義類別的起始數字 (通常是 65536)。
     */

    enum { Type = UserType + 1 };

    /**
     * @brief 覆寫 QGraphicsRectItem父親中本來就有的 type() 函式，回傳專屬的型別 ID
     */
    int type() const override { return Type; }

    // ==========================================
    // 建構子 (Constructor)
    // ==========================================
    /**
     * @brief Block 建構子
     * @param x 矩形左上角的 X 座標
     * @param y 矩形左上角的 Y 座標
     * @param w 矩形的寬度
     * @param h 矩形的高度
     */
    Block(qreal x, qreal y, qreal w, qreal h);

};

#endif // BLOCK_H
