#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <QDebug>


#include "Block.h"
#include <QGraphicsPixmapItem>


#include "Kirby.h"



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




    //小方塊敵人
    //dee = new WaddleDee();
    //dee->setPos(800, 500); // 放一個有地板的地方
    //scene->addItem(dee);

    // [修改]變成以list儲存後，一口氣生成三個實驗品
    for (int i = 0; i < 3; ++i) {
        WaddleDee *newDee = new WaddleDee();
        newDee->setPos(800 + (i * 500), 500); // 讓他們出生在不同位置
        scene->addItem(newDee);
        enemyList.append(newDee); // 把他們通通塞進清單管理
    }

    //spit星星
    connect(player, &Kirby::starFired, this, [=](StarBullet* star){
        bulletList.append(star); // 只要卡比一噴，就加進清單
        // 加這行 Debug，看看噴射時有沒有這行字出現在下方輸出視窗
        qDebug() << "Captured a star! Total stars in list:" << bulletList.size();
    });


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
// 核心遊戲迴圈~~~~~~~~~~~~~~~~
// ---------------------------------------------------------
void MainWindow::gameLoop() {
    // 1. 更新玩家狀態 (物理與動畫)
    if (player) {
        player->update();
        player->processInhale(enemyList);// [新增]：讓卡比去「吸」這群敵人
    }

    // 2. 更新敵人狀態
    // 使用迴圈統一更新，這部分你寫得很棒！
    for (Enemy *e : enemyList) {
        e->update();
    }



    // 3. 處理角色間的互動 (互動層)
    // [架構師修正]：既然有多個敵人，我們也需要用迴圈來檢查碰撞
    if (player) {
        for (Enemy *e : enemyList) {
            if (player->collidesWithItem(e)) {
                // TODO: 被撞到的後續處理（例如 player->takeDamage()）
                // 這裡目前留白，等同學來優化受擊效果
            }
        }
    }



    //吐出星星
    //for (StarBullet *b : bulletList) {
      //  if (b->isVisible()) b->update();
    //}
    // 2. [更改] 更新星星子彈
    for (int i = 0; i < bulletList.size(); ++i) {
        StarBullet *b = bulletList[i];
        b->update(); // 呼叫 StarBullet::update()，星星才會飛！

        // [進階]：如果星星飛太遠或碰到東西，記得標記為不可見
        if (b->x() < 0 || b->x() > 4860 || !b->isVisible()) {
            scene->removeItem(b);
            bulletList.removeAt(i);
            delete b;
            i--; // 索引修正
        }
    }




    // 4. 攝影機跟隨 (最後處理，因為要等所有物件座標都算好)
    if (player) {
        // [小建議]：這裡可以用 qBound 限制攝影機不超出地圖 0-4860 的範圍
        qreal camX = qBound(512.0, player->x(), 4860.0 - 512.0);
        view->centerOn(camX, 540);
    }
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
    // 5. 處理x吸氣//[add]spit star
    else if (key == Qt::Key_X) {
        //player->startInhaling();
        // [修改]：讓卡比自己決定現在是要吸還是要吐
        player->handleAttack();
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
        player->stopInhaling(); // 放開按鍵停止吸氣 //吸氣才需要release偵測
    }
}
