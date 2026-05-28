#include "HUD.h"
#include <QtGlobal>
#include <QDebug>

HUD::HUD() {
    // 1. 初始化血量文字（備援用，可隱藏）
    hpText = new QGraphicsTextItem(this); // 傳入 this，讓 hpText 成為這個群組的小孩
    hpText->setDefaultTextColor(Qt::red);
    hpText->setFont(QFont("Arial", 24, QFont::Bold));
    hpText->setVisible(false);

    // 2. 初始化 HP / Lives 圖片顯示
    hpPixmap = new QGraphicsPixmapItem(this);
    livesPixmap = new QGraphicsPixmapItem(this);
    hpPixmap->setPos(400, 0);
    livesPixmap->setPos(0, 0); // 根據圖片高度調整位置

    // 3. 預設顯示
    updateHealth(3, 3, 3, 3);

    // 4. 設定 Z 軸高度，確保 UI 永遠顯示在最上層，不會被方塊或卡比擋住
    setZValue(100);
}

void HUD::updateHealth(int currentHp, int maxHp, int currentlives, int maxlives) {
    // 如果你只有 3 張 HP 圖，則把 currentHp 限制在 1~3 範圍內
    int hpImageIndex = qBound(1, currentHp, 3);
    int livesImageIndex = qBound(0, currentlives-1, 2);

    // 調整路徑到你資源中的實際目錄，這裡是假設放在 :/Project2_Dataset/Image/HUD/
    QString hpPath = QString(":/Project2_Dataset/Image/item/Hp-%1.png").arg(hpImageIndex);
    QString livesPath = QString(":/Project2_Dataset/Image/item/lives-%1.png").arg(livesImageIndex);

    QPixmap testPixmap(hpPath);
    if (testPixmap.isNull()) {
        qDebug() << "❌ 讀取失敗！Qt 找不到這張圖，請檢查路徑或點擊點選「清除並重新構建」：" << hpPath;
    } else {
        qDebug() << "⭕ 讀取成功！圖片大小為：" << testPixmap.size();
    }

    hpPixmap->setPixmap(QPixmap(hpPath));
    livesPixmap->setPixmap(QPixmap(livesPath));

    // 備援文字仍可用於 debug 或缺圖時顯示
    hpText->setPlainText(QString("HP: %1 / %2 , Lives: %3 / %4").arg(currentHp).arg(maxHp).arg(currentlives).arg(maxlives));
}

void HUD::showGameOver() {
    // 遊戲結束時的外觀切換
    hpText->setPlainText("GAME OVER");
    hpText->setDefaultTextColor(Qt::white);
    hpText->setVisible(true);
    hpPixmap->setVisible(false);
    livesPixmap->setVisible(false);
}
