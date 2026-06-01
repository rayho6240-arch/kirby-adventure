#include "AbilityMenu.h"

AbilityMenu::AbilityMenu(QGraphicsItem *parent) : QGraphicsRectItem(parent), selectedIndex(0) {
    // 1. 設定長方形選單的大小與位置（假設遊戲解析度為 800x600，放正中央）
    setRect(0, 0, 1160, 400);
    
    // 2. 初始化有哪些能力
    abilities << AbilityType::Normal << AbilityType::Beam << AbilityType::Fire << AbilityType::Spark;

    // 3. 載入對應的能力圖示（請替換成你的實際圖片路徑）
    icons << QPixmap(":/Project2_Dataset/Image/Kirby_normal/kirby_normal.png")
          << QPixmap(":/Project2_Dataset/Image/Kirby_Beam/kirby_beam.png")
          << QPixmap(":/Project2_Dataset/Image/Kirby_fire/kirby_fire.png")
          << QPixmap(":/Project2_Dataset/Image/Kirby_spark/kirby_spark.png");

    setVisible(false); // 一開始先隱藏
    setZValue(100);    // 確保選單顯示在最上層，不會被地圖或怪物遮住
}

void AbilityMenu::moveLeft() {
    if (selectedIndex > 0) {
        selectedIndex--;
        update(); // 重新繪製選單
    }
}

void AbilityMenu::moveRight() {
    if (selectedIndex < abilities.size() - 1) {
        selectedIndex++;
        update(); // 重新繪製選單
    }
}

AbilityType AbilityMenu::getSelectedAbility() const {
    return abilities[selectedIndex];
}

void AbilityMenu::resetSelection() {
    selectedIndex = 0;
    update();
}

void AbilityMenu::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // 1. 畫出背景長方形
    painter->setBrush(QBrush(QColor(30, 30, 30, 220))); 
    painter->setPen(QPen(Qt::white, 3)); 
    painter->drawRect(rect());

    // 🎯 調整 2：定義放大後的目標尺寸（原本 40x45 放大 2 倍 -> 80x90）
    int targetW = 240;
    int targetH = 270;

    // 🎯 調整 3：重新計算排版位置，讓 4 個大圖示完美置中
    int startX = 35;   // 第一個圖示的左邊距
    int y = 65;        // 上邊距（選單高 400 - 圖示高 270 = 剩 130，上下各分 65，完美置中！）
    int spacing = 280; // 每個圖示的間隔（圖示寬 240 + 間隙 35 = 275）

    for (int i = 0; i < icons.size(); ++i) {
        int x = startX + i * spacing;

        if (!icons[i].isNull()) {
            // 🎯 調整 4：等比例放大圖片
            // 💡 這裡加上了 Qt::SmoothTransformation，可以讓放大後的圖示邊緣平滑不生硬。
            // ⚠️ 注意：如果你的遊戲是點陣像素風（Pixel Art），希望保留大顆粒的復古感，
            // 請把下面的 SmoothTransformation 改成 Qt::FastTransformation 喔！
            QPixmap scaledIcon = icons[i].scaled(targetW, targetH, 
                                                 Qt::IgnoreAspectRatio, 
                                                 Qt::SmoothTransformation);
            
            painter->drawPixmap(x, y, scaledIcon);
        } else {
            // 圖片讀取失敗時的暫代灰色方塊
            painter->setBrush(Qt::gray);
            painter->drawRect(x, y, targetW, targetH);
        }

        // 2. 畫出黃色選取框
        if (i == selectedIndex) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(QColor(255, 215, 0), 4)); // 黃金粗框
            
            // 讓黃色選取框配合長方形尺寸（ targetW 和 targetH ）
            painter->drawRect(x - 5, y - 5, targetW + 10, targetH + 10);
        }
    }
}