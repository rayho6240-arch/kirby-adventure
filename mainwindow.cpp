#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <QDebug>


#include "Block.h"
#include <QGraphicsPixmapItem>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 初始化舞台
    scene = new QGraphicsScene(this);
    //[修改] 這邊的 width 到時候要改成是 stage 1 => 4860，stage 2 => 8100
    scene->setSceneRect(0, 0, 4860, 1080);
    scene->setBackgroundBrush(Qt::white);


    // 2. 初始化卡比 (交給 Kirby 類別處理細節)
    player = new Kirby();
    player->setPos(400, 100);
    scene->addItem(player); //演員上台

    // 3. 設定攝影機
    view = new QGraphicsView(scene, this);  //在家，畫一幅畫
    setCentralWidget(view);   //把這幅畫塞進畫框的最中心，並且填滿它。
    //[新增] 固定這幅畫的大小為1620*1080 pixels
    view->setFixedSize(1620, 1080);

    // [新增] 初始化雙擊計時器(衝刺跑)
    doubleTapTimer = new QTimer(this);
    doubleTapTimer->setSingleShot(true); // 設為只觸發一次
    connect(doubleTapTimer, &QTimer::timeout, this, &MainWindow::onDoubleTapTimerTimeout);

    // 4. 設定遊戲計時器
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::gameLoop); //內建函數: 當 '鬧鐘' '時間到了'，請叫 '我(this)' '去執行 gameLoop。'
    timer->start(16);

    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocus();                     //this 就是「這個 MainWindow 視窗本人」。


    // --- 1. 鋪設背景大圖 ---
    // 修正路徑以符合新的資源資料夾結構：:/Project2_Dataset/Image/background/
    // 假設每張圖寬度是 1620，我們把三張圖橫向接在一起

    QGraphicsPixmapItem* bg1 = new QGraphicsPixmapItem(QPixmap(":/Project2_Dataset/Image/background/Stage1(1).png"));
    bg1->setPos(0, 890); // 第一張圖在最左邊
    scene->addItem(bg1);

    QGraphicsPixmapItem* bg2 = new QGraphicsPixmapItem(QPixmap(":/Project2_Dataset/Image/background/Stage1(2).png"));
    bg2->setPos(1620, 390); // 第二張圖接在後面
    scene->addItem(bg2);

    QGraphicsPixmapItem* bg3 = new QGraphicsPixmapItem(QPixmap(":/Project2_Dataset/Image/background/Stage1(3).png"));
    bg3->setPos(3240, 800); // 第三張圖
    scene->addItem(bg3);


    // 設定背景圖在最後面
    bg1->setZValue(-10);
    bg2->setZValue(-10);
    bg3->setZValue(-10);

    // --- 鋪設隱形碰撞層 ---
    // 你必須根據圖片上「真正可以踩」的位置，來設定這些 Block 的座標
    // 以下數值 (x, y, 寬, 高) 只是舉例，你需要自己去抓圖片的地板高度
    Block* ground1 = new Block(0, 900, 1620, 400); // 第一張圖的地板
    scene->addItem(ground1);

    Block* ground2 = new Block(1620, 950, 1000, 130); // 第二張圖的地板可能有高低差
    scene->addItem(ground2);

    Block* wall1 = new Block(2620, 800, 100, 150); // 第二張圖中間突起的那塊牆壁
    scene->addItem(wall1);

}


MainWindow::~MainWindow() {
    delete ui;
}


// ---------------------------------------------------------
// [新增 Slot 函數實作]
// ---------------------------------------------------------
void MainWindow::onDoubleTapTimerTimeout() {
    lastReleasedKey = -1; // 時間到，清空上一次按鍵，雙擊無效
}

// ---------------------------------------------------------
// 核心遊戲迴圈
// ---------------------------------------------------------
void MainWindow::gameLoop() {
    // 讓卡比自己去更新他的物理跟動畫
    player->update();

    // 5. 攝影機跟隨
    view->centerOn(player->x(), 540);
    //x() 是 QGraphicsPixmapItem（以及所有 Qt 圖形元件）內建的一個成員函數。
    //回傳該物件目前在「舞台（Scene）」上的 X 座標（水平位置）。
}

// ---------------------------------------------------------
// 按鈕模塊
//---------------------------------------------------------
//把簡單的 if (event->key() == Qt::Key_Left) vx = -7; ... 換成複雜的衝刺判定
//加入按鈕防止彈跳，雙擊有衝刺功能
// =========================================================
void MainWindow::keyPressEvent(QKeyEvent *event) {
    // [關鍵新增] 防彈跳：如果是作業系統長按產生的連發訊號，直接忽略
    if (event->isAutoRepeat()) return;

    int key = event->key();

    // 1. 處理蹲下
    if (key == Qt::Key_Down) {
        player->setDown(true);
        player->setDashing(false);
    }
    // 2. 處理跳躍與飛行
    else if (key == Qt::Key_Z) {
        player->jump();
    }
    else if (key == Qt::Key_Up) {
        player->fly();
    }
    // 3. 處理右鍵
    else if (key == Qt::Key_Right) {
        if (doubleTapTimer->isActive() && lastReleasedKey == Qt::Key_Right) {
            player->setDashing(true);
            player->setHorizontalVelocity(12); // DASH_SPEED
            doubleTapTimer->stop();
            lastReleasedKey = -1;
        } else {
            if (!player->getVx()) player->setHorizontalVelocity(7);
        }
    }
    // 4. 處理左鍵
    else if (key == Qt::Key_Left) {
        if (doubleTapTimer->isActive() && lastReleasedKey == Qt::Key_Left) {
            player->setDashing(true);
            player->setHorizontalVelocity(-12); // -DASH_SPEED
            doubleTapTimer->stop();
            lastReleasedKey = -1;
        } else {
            if (!player->getVx()) player->setHorizontalVelocity(-7);
        }
    }
    // 5. 處理x吸氣
    else if (key == Qt::Key_X) {
        player->startInhaling();
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    // [關鍵新增] 防彈跳：過濾掉作業系統產生的「假放開」
    if (event->isAutoRepeat()) return;

    int key = event->key();

    if (key == Qt::Key_Down) {
        player->setDown(false);
    }
    else if (key == Qt::Key_Left || key == Qt::Key_Right) {
        player->setDashing(false);
        player->setHorizontalVelocity(0);

        // 啟動雙擊判定計時器
        lastReleasedKey = key;
        doubleTapTimer->start(DOUBLE_TAP_WINDOW);
    }
    else if (key == Qt::Key_X) {
        player->stopInhaling(); // 放開按鍵停止吸氣
    }
}
