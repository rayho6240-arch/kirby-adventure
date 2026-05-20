#include "HUD.h"

HUD::HUD() {
    // 1. 初始化血量文字
    hpText = new QGraphicsTextItem(this); // 傳入 this，讓 hpText 成為這個群組的小孩
    hpText->setDefaultTextColor(Qt::red);
    hpText->setFont(QFont("Arial", 24, QFont::Bold));

    // 2. 預設顯示
    updateHealth(6, 6, 3, 3);

    // 3. 設定 Z 軸高度，確保 UI 永遠顯示在最上層，不會被方塊或卡比擋住
    setZValue(100);
}

void HUD::updateHealth(int currentHp, int maxHp, int currentlives, int maxlives) {
    // 只要呼叫這個函式，就會自動更新文字內容
    hpText->setPlainText(QString("HP: %1 / %2 , Lives: %3 / %4").arg(currentHp).arg(maxHp).arg(currentlives).arg(maxlives));
    //TODO: 圖片弄好
}

void HUD::showGameOver() {
    // 遊戲結束時的外觀切換
    hpText->setPlainText("GAME OVER");
    hpText->setDefaultTextColor(Qt::white);
}
