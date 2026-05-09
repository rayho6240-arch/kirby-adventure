#ifndef STARBULLET_H
#define STARBULLET_H
//卡比吸入敵人或方塊後，按下攻擊鍵「吐出來的星星子彈」。負責處理子彈的直線飛行、速度，以及打到敵人或牆壁時的爆炸特效。


#include <QGraphicsPixmapItem>
#include <QObject>

class StarBullet : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    StarBullet(qreal x, qreal y, bool toRight);
    void update(); // 由 MainWindow 的迴圈呼叫

private:
    qreal vx;
};



#endif // STARBULLET_H
