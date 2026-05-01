#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 初始化舞台
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 5000, 1080);
    scene->setBackgroundBrush(Qt::white);

    // 簡單地板測試
    for (int i = 0; i < 10; i++) {
        QGraphicsRectItem *ground = new QGraphicsRectItem(i * 300, 850, 200, 50);
        ground->setBrush(Qt::darkGray);
        scene->addItem(ground);
    }

    // 2. 初始化卡比 (注意：這裡不要重複宣告 QGraphicsPixmapItem*)
    QPixmap kirbyImg(":/Project2_Dataset/Image/Kirby_normal/kirby_stop_R.png");
    kirby = new QGraphicsPixmapItem(kirbyImg);
    kirby->setPos(400, 100);
    scene->addItem(kirby);

    // 3. 設定攝影機
    view = new QGraphicsView(scene, this);
    setCentralWidget(view);

    // 4. 設定遊戲計時器
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::gameLoop);
    timer->start(16);

    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocus();
}

// ---------------------------------------------------------
// 萬用換圖函數：處理各種檔名規律
// ---------------------------------------------------------
void MainWindow::updateKirbySprite(QString action, QString dir, int frame) {
    QString path;

    if (action == "jump") {
        // 針對 kirby_jump(1).png
        path = QString(":/Project2_Dataset/Image/Kirby_normal/kirby_jump(%1).png").arg(frame);
    }
    else if (frame == 0) {
        // 針對 kirby_stop_R.png
        path = QString(":/Project2_Dataset/Image/Kirby_normal/kirby_%1_%2.png").arg(action).arg(dir);
    }
    else {
        // 針對 kirby_run_1_R.png
        path = QString(":/Project2_Dataset/Image/Kirby_normal/kirby_%1_%2_%3.png").arg(action).arg(frame).arg(dir);
    }

    QPixmap pix(path);
    if (!pix.isNull()) {
        kirby->setPixmap(pix);
    }
}

// ---------------------------------------------------------
// 核心遊戲迴圈
// ---------------------------------------------------------
void MainWindow::gameLoop() {
    // 1. 物理運算
    vy += gravity;
    kirby->setPos(kirby->x() + vx, kirby->y() + vy);

    // 2. 落地判定
    if (kirby->y() >= 800) {
        kirby->setY(800);
        vy = 0;
    }

    // 3. 更新朝向
    if (vx > 0) isFacingRight = true;
    else if (vx < 0) isFacingRight = false;
    QString dir = isFacingRight ? "R" : "L";

    // 4. 根據狀態播放動畫
    if (kirby->y() < 800) {
        // 空中：跳躍圖
        updateKirbySprite("jump", dir, 1);
    }
    else if (vx != 0) {
        // 地面移動：跑圖
        frameCounter++;
        int runFrame = (frameCounter / 5) % 4 + 1;
        updateKirbySprite("run", dir, runFrame);
    }
    else {
        // 地面靜止：站立圖
        frameCounter = 0;
        updateKirbySprite("stop", dir);
    }

    // 5. 攝影機跟隨
    view->centerOn(kirby->x(), 540);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Left) vx = -7;
    else if (event->key() == Qt::Key_Right) vx = 7;
    else if (event->key() == Qt::Key_Up && kirby->y() >= 800) vy = -12;
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) vx = 0;
}

MainWindow::~MainWindow() {
    delete ui;
}
