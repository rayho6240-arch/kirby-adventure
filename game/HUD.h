#ifndef HUD_H
#define HUD_H

#include <QGraphicsItemGroup>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QFont>
#include <QString>

/**
 * @brief 遊戲介面類別 (Heads-Up Display)
 * @details 負責在畫面上顯示血量、分數、Game Over 等資訊。
 */
class HUD : public QGraphicsItemGroup {
public:
    HUD();

    // 給外界 (MainWindow) 呼叫的介面
    void updateHealth(int currentHp, int maxHp, int currentlives, int maxlives);
    void showGameOver();

private:
    QGraphicsTextItem *hpText; ///< 保留文字作為備援顯示
    QGraphicsPixmapItem *hpPixmap; ///< 用來顯示 HP 圖片
    QGraphicsPixmapItem *livesPixmap; ///< 用來顯示 Lives 圖片
};

#endif // HUD_H
