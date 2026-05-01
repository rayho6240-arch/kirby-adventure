#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 初始化舞台
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 5000, 1080);
    scene->setBackgroundBrush(Qt::white); // 強制背景為白色，比較好觀察


    // 多放幾個方塊
    for (int i = 0; i < 10; i++) {
        QGraphicsRectItem *ground = new QGraphicsRectItem(i * 300, 850, 200, 50);
        ground->setBrush(Qt::darkGray);
        scene->addItem(ground);
    }

    // 2. 初始化卡比
    QPixmap kirbyImg(":/Project2_Dataset/Image/Kirby_normal/kirby_stop_R.png");
    kirby = new QGraphicsPixmapItem(kirbyImg);
    kirby->setPos(400, 100); // 讓它從高一點的地方開始掉
    scene->addItem(kirby);

    // 3. 設定攝影機
    view = new QGraphicsView(scene, this);
    setCentralWidget(view);

    // 4. 設定遊戲計時器 (60 FPS)
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::gameLoop);
    timer->start(16); // 每 16ms 跑一次循環

    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocus();

}

void MainWindow::gameLoop() {
    // === 1. 物理與移動計算 ===
    vy += gravity;
    float nextY = kirby->y() + vy;
    float nextX = kirby->x() + vx;

    // === 2. 邊界與碰撞判定 ===
    float sceneWidth = scene->sceneRect().width();
    float kirbyWidth = 50;

    // 角色水平撞牆判定
    if (nextX < 0) nextX = 0;
    if (nextX > sceneWidth - kirbyWidth) nextX = sceneWidth - kirbyWidth;

    // 角色地板判定
    if (nextY > 800) {
        nextY = 800;
        vy = 0;
    }

    // 正式更新方塊位置
    kirby->setPos(nextX, nextY);

    // === 新增：卡比動畫處理 ===
    if (vx != 0) {
        frameCounter++;
        // 每 5 幀換一張圖，循環 1~4 幀
        int runFrame = (frameCounter / 5) % 4 + 1;
        QString dir = (vx > 0) ? "R" : "L"; // 判斷往左還是往右

        // 動態組合路徑，例如：:/Project2_Dataset/Image/Kirby_normal/kirby_run_1_R.png
        QString path = QString(":/Project2_Dataset/Image/Kirby_normal/kirby_run_%1_%2.png")
                        .arg(runFrame).arg(dir);
        kirby->setPixmap(QPixmap(path));
    }
    else {
        // 沒動時，顯示站立圖 (這裡先預設 R，你也可以記住最後的方向)
        kirby->setPixmap(QPixmap(":/Project2_Dataset/Image/Kirby_normal/kirby_stop_R.png"));
    }




    // === 3. 攝影機捲動 (Camera Follow) ===
    float viewWidth = view->viewport()->width();
    float centerX = kirby->x();

    // 鏡頭邊界鎖定 (防止拍到黑影)
    if (centerX < viewWidth / 2) {
        centerX = viewWidth / 2;
    }
    if (sceneWidth > viewWidth) {
        if (centerX > sceneWidth - viewWidth / 2) {
            centerX = sceneWidth - viewWidth / 2;
        }
    } else {
        centerX = sceneWidth / 2;
    }

    // 更新鏡頭位置
    view->centerOn(centerX, 540);
}


MainWindow::~MainWindow()
{
    delete ui;
}


// 按下按鍵時：設定速度
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Left) {
        vx = -7;  // 往左走
    } else if (event->key() == Qt::Key_Right) {
        vx = 7;   // 往右走
    } else if (event->key() == Qt::Key_Up) {
        if (kirby->y() >= 800) { // 只有在地上才能跳
            vy = -12;
        }
    }
}

// 放開按鍵時：速度歸零
void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
        vx = 0;
    }
}
