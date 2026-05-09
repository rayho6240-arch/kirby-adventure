#include "StarBullet.h"

StarBullet::StarBullet(qreal x, qreal y, bool toRight) {
    setPixmap(QPixmap(":/Project2_Dataset/Image/Kirby_normal/kirby_attack_star_L(1).png").scaled(40, 40)); // 假設你有這張圖
    setPos(x, y);
    vx = toRight ? 15.0 : -15.0; // 星星彈射速度非常快
}

void StarBullet::update() {
    setX(x() + vx);
    // 超出地圖邊界就自我毀滅（這裡先簡單處理，之後可以加進清單刪除）(或是撞到方快就消失)
    if (x() < 0 || x() > 4860) setVisible(false);
}
