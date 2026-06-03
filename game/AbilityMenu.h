#ifndef ABILITYMENU_H
#define ABILITYMENU_H

#include <QGraphicsRectItem>
#include <QPainter>
#include <QVector>
#include <QPixmap>

// 假設這是你的能力列舉（請根據你實際的 Kirby::Form 修改）
enum class AbilityType {
    Normal,
    Beam,
    Fire,
    Spark
};

class AbilityMenu : public QGraphicsRectItem {
public:
    AbilityMenu(QGraphicsItem *parent = nullptr);

    // 控制選擇游標
    void moveLeft();
    void moveRight();
    AbilityType getSelectedAbility() const;
    void resetSelection();
    void setUnlocked(int index, bool value) {unlocked[index] = value;} // 用來解鎖能力的簡單函式
    bool getUnlocked(int index) const { return unlocked[index]; } // 檢查能力是否解鎖
    QVector<bool> getUnlockedStatus() const { return unlocked; } // 取得所有能力的解鎖狀態
    void setUnlockedStatus(const QVector<bool>& status) { unlocked = status; } // 設定所有能力的解鎖狀態

protected:
    // 🛑 核心：複寫 paint，用來畫出長方形背景、圖示和選取框
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    int selectedIndex;
    QVector<AbilityType> abilities;
    QVector<QPixmap> icons; // 儲存能力的圖示
    QVector<bool> unlocked = {true, false, false, false}; // 儲存能力是否解鎖的狀態
};

#endif // ABILITYMENU_H