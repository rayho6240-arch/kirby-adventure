#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QTimer>

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
    QGraphicsRectItem *kirby;
    QTimer *timer;

    // 簡單的物理變數
    float vx = 0;       // 水平速度
    float vy = 0;       // 垂直速度
    float gravity = 0.5; // 重力加速度

protected:
    // 讓視窗能夠「聽」到鍵盤的動作
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

};

#endif // MAINWINDOW_H
