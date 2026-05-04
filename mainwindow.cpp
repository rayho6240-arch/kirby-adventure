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
        QGraphicsRectItem *ground = new QGraphicsRectItem(i * 300, 850, 200, 50);  // 創立演員，把圖片給他，還沒上台
        ground->setBrush(Qt::darkGray); //給演員化妝
        scene->addItem(ground);  //演員上台
    }

    // 2. 初始化卡比
    QPixmap kirbyImg(":/Project2_Dataset/Image/Kirby_normal/kirby_stop_R.png"); // 1. 載入圖片資料
    kirby = new QGraphicsPixmapItem(kirbyImg);    // 2. 創立演員，把圖片給他，還沒上台
    kirby->setPos(400, 100);
    scene->addItem(kirby); //演員上台

    // 3. 設定攝影機
    view = new QGraphicsView(scene, this);  //在家，畫一幅畫
    setCentralWidget(view);   //把這幅畫塞進畫框的最中心，並且填滿它。

    // 4. 設定遊戲計時器
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::gameLoop); //內建函數: 當 '鬧鐘' '時間到了'，請叫 '我(this)' '去執行 gameLoop。'
    timer->start(16);

    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocus();                     //this 就是「這個 MainWindow 視窗本人」。

}



// ---------------------------------------------------------
// 萬用換圖函數：處理各種檔名規律
// ---------------------------------------------------------

//靠 gameLoop 裡的 「計時器」 加上 「取餘數（%）」 運算。
//第一個 .arg(action)：它會掃描字串，找到第一個出現的 %1，然後把 action 變數的內容（例如 "run"）塞進去。
//第二個 .arg(frame)：它會找 %2，把計算好的數字（例如 1）塞進去。
//第三個 .arg(dir)：它會找 %3，把方向（例如 "R"）塞進去。
//==============================================================

void MainWindow::updateKirbySprite(QString action, QString dir, int frame) {
    QString path;

    if (action == "jump") {
        // 針對 kirby_jump(1).png
        path = QString(":/Project2_Dataset/Image/Kirby_normal/kirby_jump(%1).png").arg(frame);
    }
    else if (frame == 0) {
        // 針對 kirby_stop_R.png
        path = QString(":/Project2_Dataset/Image/Kirby_normal/kirby_%1_%2.png").arg(action).arg(dir);//替換的東西是外面傳進來的
    }
    else {
        // 針對 kirby_run_1_R.png
        path = QString(":/Project2_Dataset/Image/Kirby_normal/kirby_%1_%2_%3.png").arg(action).arg(frame).arg(dir);
    }

    QPixmap pix(path);   //是一個obj， pix以(path)為ctor
    if (!pix.isNull()) { //再次檢查，前面邏輯給出的地址不是空的
        kirby->setPixmap(pix); //把這個道具給演員'kirby'
    }
}




// ---------------------------------------------------------
// 核心遊戲迴圈
// ---------------------------------------------------------
void MainWindow::gameLoop() {
    // 1. 物理運算
    vy += gravity;
    // --- 修改開始：邊界檢查邏輯 ---
    qreal nextX = kirby->x() + vx;
    qreal nextY = kirby->y() + vy;

    // 左邊界檢查 (0)
    if (nextX < 0) {
        nextX = 0;
    }
    // 右邊界檢查 (5000 - 卡比寬度)
    // 使用內建函數 boundingRect().width() 回傳:能把卡比「框起來」的最小長方形寬度。
    else if (nextX > 5000 - kirby->boundingRect().width()) {
        nextX = 5000 - kirby->boundingRect().width();
    }

    // 套用位置
    kirby->setPos(nextX, nextY);


    // 2. 落地判定 (或將 800 改為變數以便維護)
    if (kirby->y() >= 800) {
        kirby->setY(800);
        vy = 0;
    }

    // 3. 更新朝向
    if (vx > 0) isFacingRight = true;
    else if (vx < 0) isFacingRight = false;
    QString dir = isFacingRight ? "R" : "L"; //宣告+定義變數 dir(direction)

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
    //x() 是 QGraphicsPixmapItem（以及所有 Qt 圖形元件）內建的一個成員函數。
    //回傳該物件目前在「舞台（Scene）」上的 X 座標（水平位置）。
}



// ---------------------------------------------------------
// 按鈕模塊
// ---------------------------------------------------------
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Left) vx = -7;
    else if (event->key() == Qt::Key_Right) vx = 7;
    else if (event->key() == Qt::Key_Up && kirby->y() >= 800) vy = -12;
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) vx = 0;
}






// ---------------------------------------------------------
// dtor
// ---------------------------------------------------------
MainWindow::~MainWindow() {
    delete ui;
}
