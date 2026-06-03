#ifndef WADDLEDEE_H
#define WADDLEDEE_H
#include <QString>
#include <QPixmap>

// ==========================================
// 1. 基底類別引入
// ==========================================
#include "Enemy.h"

/**
 * @brief WaddleDee 瓦豆魯迪 (一般步兵敵人)
 * @details 繼承自 Enemy。這是最基礎的敵人，通常只會在地面上左右來回走動。
 * 由於共用的物理碰撞邏輯已經在 Enemy 中實作，這個類別只需要專注於
 * 自身的初始數值設定與專屬動畫更新即可。
 */
class WaddleDee : public Enemy {
public:
    // ==========================================
    // 生命週期與核心介面 (Lifecycle & Interface)
    // ==========================================

    /**
     * @brief WaddleDee 建構子
     * @param parent 傳遞給 QGraphicsPixmapItem 的父物件
     * 如果呼叫這個函式的人沒有提供參數，就預設為 nullptr（空指標，代表沒有爸爸)
     */
    WaddleDee(QGraphicsItem *parent = nullptr);

    /**
     * @brief 實作 WaddleDee 專屬的每幀更新邏輯
     * @note 這裡強制覆寫 (override) 了 Enemy 定義的純虛擬函式。
     * 預期在實作檔 (.cpp) 裡，會呼叫 handlePhysics() 來處理移動，
     * 並負責切換 WaddleDee 的左右走路動畫圖片。
     */
    void update() override;

private:
    // [新增]動畫渲染
    void updateSprite();
    int frameCounter = 0;
};


#endif // WADDLEDEE_H



//關於cpp中創立物件special case(future)
//Enemy *waddle = new WaddleDee(someBossItem);
//如果你在括號裡傳入了某個大魔王物件，這隻 Waddle Dee 就會變成大魔王的小孩。魔王死掉，它會跟著死掉。
