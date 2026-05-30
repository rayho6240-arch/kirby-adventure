#include "StarBullet.h"
#include <QGraphicsScene>
#include <QList>
#include "Block.h"
// =========================================================
// 1. 初始化與建構子 (Initialization)
// =========================================================

/**
 * @brief StarBullet 建構子
 * @param x 生成位置 X
 * @param y 生成位置 Y
 * @param toRight 面朝方向 (決定子彈向左飛或向右飛)
 */
StarBullet::StarBullet(qreal x, qreal y, bool toRight) {
    /*// 1. 載入並設定星星圖片，同時將其強制縮放為 40x40 大小
    // @note 這裡目前先統一用向左的圖片。未來可增加素材L 或是利用 QImage::mirrored
    setPixmap(QPixmap(":/Project2_Dataset/Image/Kirby_normal/kirby_attack_star_L(1).png").scaled(141.53,100));

    // 2. 設定出生座標 (由 MainWindow 呼叫時，傳入卡比的當前座標)
    setPos(x, y);

    // 3. 決定飛行方向與速度
    // 如果 toRight 為 true，vx = 15.0；否則 vx = -15.0。星星射速度非常快！
    vx = toRight ? 15.0 : -15.0;
    */

    // [修改] 用toRight判斷星星圖片以及方向
    if(toRight){
        setPixmap(QPixmap(":/Project2_Dataset/Image/Kirby_normal/kirby_attack_star_R(1).png").scaled(141.53,100));
        setPos(x, y);
        vx = 15.0;
    }
    else{
        setPixmap(QPixmap(":/Project2_Dataset/Image/Kirby_normal/kirby_attack_star_L(1).png").scaled(141.53,100));
        setPos(x, y);
        vx = -15.0; 
    }
}

// =========================================================
// 2. 每幀更新邏輯 (Frame Update)
// =========================================================

/**
 * @brief 子彈的飛行與碰撞檢查
 */
void StarBullet::update() {
    // --- 1. 直線物理移動 ---
    setX(x() + vx);

    // --- 2. 邊界清除檢查 (Garbage Collection 機制雛形) ---
    // 超出地圖邊界就自我毀滅
    if (x() < 0 || x() > 4860) {
        // @note 這裡先簡單處理隱藏。
        // 正式的做法：在 .h 檔宣告一個 `bool isDestroyed = false;`，
        // 這裡將其設為 true。然後交給 MainWindow 的 gameLoop 把這顆子彈徹底 delete。
        setVisible(false);
    }

    // --- 3. 實體碰撞檢查 (預留區) ---
    // TODO: 之後可以在這裡加上 scene()->collidingItems(this) 的檢查。
    // 1. 如果碰到 Block (qgraphicsitem_cast<Block*>) -> 隱藏自己 / 播放爆炸特效
    // 2. 如果碰到 Enemy (qgraphicsitem_cast<Enemy*>) -> 殺死敵人 -> 隱藏自己
    QList<QGraphicsItem *> colliding = scene()->collidingItems(this);
    for (QGraphicsItem *item : colliding) {
        if (item == this) continue;

        Block *block = dynamic_cast<Block *>(item);
        if (block) {
            setVisible(false);
            deleteLater();
            return;
        }
    }
}
