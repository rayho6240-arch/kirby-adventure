#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QTimer>

// [新增] 引入我們剛剛寫好的卡比類別
#include "Kirby.h"
#include "WaddleDee.h"
//[新增] 一個敵人變軍隊
#include <QList>
#include "StarBullet.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots: //slots 讓這個函數可以被 connect() 連結。//像「收音機」一樣，監聽某個「信號（Signal）」。//這邊用來監視time
    void gameLoop(); // 遊戲每一幀要做的事
    void onDoubleTapTimerTimeout();  // 計時器時間到時的slot

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QTimer *timer;




    //WaddleDee *dee;
    QList<Enemy*> enemyList; //把一個敵人變成軍隊，用 list儲存

    QList<StarBullet*> bulletList;





    // [修改] 原本的 QGraphicsPixmapItem *kirby; 變成了我們自定義的 Kirby 類別
    Kirby *player;

    // --- 鍵盤雙擊判定保留在 MainWindow，因為這是玩家輸入層級的邏輯 ---
    int lastReleasedKey = -1;        // 記錄上一個放開的方向鍵
    QTimer *doubleTapTimer;          // 雙擊判定計時器
    const int DOUBLE_TAP_WINDOW = 250; // 判定時間差（250毫秒內連按算衝刺）


protected:
    // 讓視窗能夠「聽」到鍵盤的動作
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;


};

#endif // MAINWINDOW_H
