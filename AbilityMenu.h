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
    void setBeamUnlocked(bool unlocked) { isBeamUnlocked = unlocked; }
    void setFireUnlocked(bool unlocked) { isFireUnlocked = unlocked; }
    void setSparkUnlocked(bool unlocked) { isSparkUnlocked = unlocked; }

protected:
    // 🛑 核心：複寫 paint，用來畫出長方形背景、圖示和選取框
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    int selectedIndex;
    QVector<AbilityType> abilities;
    QVector<QPixmap> icons; // 儲存能力的圖示

    bool isBeamUnlocked = false;
    bool isFireUnlocked = false;
    bool isSparkUnlocked = false;
};

#endif // ABILITYMENU_H