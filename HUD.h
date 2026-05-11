#ifndef HUD_H
#define HUD_H

#include <QGraphicsItemGroup>
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
    void updateHealth(int currentHp, int maxHp);
    void showGameOver();

private:
    QGraphicsTextItem *hpText; ///< 負責顯示血量的文字物件
};

#endif // HUD_H
