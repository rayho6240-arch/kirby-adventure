#ifndef WADDLEDEE_H
#define WADDLEDEE_H

#include "Enemy.h"

class WaddleDee : public Enemy {
public:
    WaddleDee(QGraphicsItem *parent = nullptr);
    void update() override; // 實作更新邏輯
};

#endif
