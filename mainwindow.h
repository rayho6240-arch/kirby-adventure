#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QTimer>

#include <QGraphicsPixmapItem>

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
    QGraphicsPixmapItem *kirby;

    int frameCounter = 0;        // 用來算動畫幀
    int flyFrame = 1; // 用來記錄目前是第一張還是第二張翅膀圖


    bool isFacingRight = true; //  //不動時朝左或朝右，預設朝右
    bool isDown = false;  // 是否蹲下
    bool isFlying = false; // 是否飛行



    bool isDashing = false;  // 是否正在衝刺
    int lastReleasedKey = -1;        // 記錄上一個放開的方向鍵
    QTimer *doubleTapTimer;          // 雙擊判定計時器
    const int DOUBLE_TAP_WINDOW = 250; // 判定時間差（250毫秒內連按算衝刺）
    const qreal DASH_SPEED = 14;      // 衝刺速度（原走路速度 7 的兩倍）


    // 簡單的物理變數
    float vx = 0;       // 水平速度
    float vy = 0;       // 垂直速度
    float gravity = 0.5; // 重力加速度


    enum State { Standing, Running, Jumping, Flying, Crouching, Attacking };
    State currentState = Standing;


    // 換圖函數宣告
    void updateKirbySprite(QString action, QString dir, int frame = 0);






protected:
    // 讓視窗能夠「聽」到鍵盤的動作
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

};

#endif // MAINWINDOW_H
