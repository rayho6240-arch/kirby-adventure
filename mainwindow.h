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

private slots:
    void gameLoop(); // 遊戲每一幀要做的事


private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QTimer *timer;

    QGraphicsPixmapItem *kirby;  // 換成這個
    int frameCounter = 0;        // 新增：用來算動畫幀
    bool isFacingRight = true; //  //不動時朝左或朝右，預設朝右

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
