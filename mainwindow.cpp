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

}



// ---------------------------------------------------------
// [新增 Slot 函數實作]
// ---------------------------------------------------------
void MainWindow::onDoubleTapTimerTimeout() {
    lastReleasedKey = -1; // 時間到，清空上一次按鍵，雙擊無效
}




// ---------------------------------------------------------
// 萬用換圖函數：處理各種檔名規律
// ---------------------------------------------------------

//靠 gameLoop 裡的 「計時器」 加上 「取餘數（%）」 運算。
//第一個 .arg(action)：它會掃描字串，找到第一個出現的 %1，然後把 action 變數的內容（例如 "run"）塞進去。
//第二個 .arg(frame)：它會找 %2，把計算好的數字（例如 1）塞進去。
//第三個 .arg(dir)：它會找 %3，把方向（例如 "R"）塞進去。
//新增水平翻轉，最好還是透過改檔名跟截圖素材比較好，但就是紀錄一下有這種工具pix.toImage().mirrored(true, false);
//
//
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

        // [新增] 如果是跳躍圖片，且面向左邊 (dir == "L")，進行水平翻轉
        if (action == "jump" && dir == "L") {
            // toImage() 轉成可編輯格式，mirrored(true, false) 代表水平翻轉
            QImage flippedImage = pix.toImage().mirrored(true, false);
            pix = QPixmap::fromImage(flippedImage); // 轉換回 QPixmap
        }

        kirby->setPixmap(pix); //把這個道具給演員'kirby'
    }
}


// ---------------------------------------------------------
// 核心遊戲迴圈
// ---------------------------------------------------------
//vx 是「玩家的意圖」，而 currentVx 是「物理引擎的最終裁決」
//vx 是private變數， currentVx 是區域變數，用來算nextX
//
//
//
//==============================================================

void MainWindow::gameLoop() {
    // 1. 物理運算
    if (isFlying) {
        vy += (gravity * 0.65); // 飛行時重力只有原本的 50%
    } else {
        vy += gravity;
    }

    // 蹲下時不准左右移動
    // 如果 isDown 為 true，這幀的速度就是 0，否則維持 vx
    qreal currentVx = (isDown || isInhaling) ? 0 : vx;

    // --- 邊界檢查邏輯 ---
    // 使用 qBound 確保 nextX 永遠在 [0, 舞台寬度 - 卡比寬度] 之間
    // [修改] 我們不再需要這裡的獨立 if判斷，因為 setPos 時會自動攔截
    qreal nextX = qBound(0.0, kirby->x() + currentVx, 5000.0 - kirby->boundingRect().width());
    qreal nextY = kirby->y() + vy;

    // [修改] 下面的 if 攔截可以註解掉了，因為 qBound 已經處理好
    /*
    // 左邊界檢查 (0)
    if (nextX < 0) {
        nextX = 0;
    }
    // 右邊界檢查 (5000 - 卡比寬度)
    // 使用內建函數 boundingRect().width() 回傳:能把卡比「框起來」的最小長方形寬度。
    else if (nextX > 5000 - kirby->boundingRect().width()) {
        nextX = 5000 - kirby->boundingRect().width();
    }
    */

    // 套用位置
    kirby->setPos(nextX, nextY);


    // 2. 落地判定 (或將 800 改為變數以便維護)
    if (kirby->y() >= 800) {
        kirby->setY(800);
        vy = 0;
        isFlying = false; // [新增] 落地自動取消飛行狀態
        // isFlying動畫裡的 flyFrame 落地重置在你的 keyPress 邏輯中處理即可，這裡不需要重複。
        isFlying = false; // [關鍵新增] 一旦碰到地面，強制解除變胖/飛行狀態
    }

    // 3. 更新朝向
    // 就算蹲下不能走，原地按左右鍵還是可以轉頭（因為 vx 還是有值）
    if (vx > 0) isFacingRight = true;
    else if (vx < 0) isFacingRight = false;
    QString dir = isFacingRight ? "R" : "L"; //宣告+定義變數 dir(direction)

    // 4. 根據狀態播放動畫
    // [修改] 這裡的 if-else 順序非常重要！決定了動畫的「優先權」後續可能用 RTOS架構之類的
    if (isDown && kirby->y() >= 800) {
        // [新增] 地面蹲下：蹲下圖 (優先級最高，蹲下就不能播跑步)
        updateKirbySprite("down", dir, 0); // 傳入 0 讓它走你單張圖的邏輯
    }

    else if (isInhaling) {
        // [新增] 吸氣動畫優先級也很高！
        updateKirbySprite("attack", dir, 0); // 替換成你的吸氣圖片名稱
    }

    else if (isFlying) {
        //空中飛行：這裡直接顯示 flyFrame（這個變數會在你的按一下拍一次 keyPressEvent 裡被切換）
        //updateKirbySprite("fly", dir, flyFrame); 修改掉了
        // [修改] 飛行時的動畫判斷
        if (flapCounter > 0) {
            flapCounter--; // 計數器遞減
            updateKirbySprite("fly", dir, 2); // 顯示翅膀拍下的狀態 (第 2 張)
        } else {
            updateKirbySprite("fly", dir, 1); // 沒按按鍵時，維持滑翔/展翅狀態 (第 1 張)
        }
    }

    //新增下落動畫
    else if (kirby->y() < 800) {
        // [修改] 區分空中上升與下落
        if (vy > 0) {
            // 下落中 (vy > 0)：播放下落動畫 (第 3 張圖)
            updateKirbySprite("jump", dir, 3);
        } else {
            // 上升中 (vy <= 0)：播放跳躍動畫 (第 1 張圖)
            updateKirbySprite("jump", dir, 1);
        }
    }
    // --- 修改：衝刺動畫狀態 ---
    else if (isDashing && vx != 0) {
        // [衝刺] 地面衝刺：我們讓標準跑圖循環快兩倍
        // 如果你有專用的衝刺圖（如 kirby_run_4，但大多卡比遊戲衝刺只是跑快點的圖），
        // 這裡可以寫 `updateKirbySprite("run", dir, 4)` 恆定顯示第4幀，
        // 或者使用 `(frameCounter / 3) % 4 + 1` 讓標準跑循環快兩倍。
        // 基於你給的圖kirby_run_4，我們恆定顯示衝刺圖的單幀手感通常更好：
        frameCounter++;
        int runframe = (frameCounter / 5) % 2 + 4;
        updateKirbySprite("run", dir, runframe);
    }
    else if (vx != 0) {
        // 地面移動：跑圖
        frameCounter++;
        int runFrame = (frameCounter / 10) % 3 + 1;
        updateKirbySprite("run", dir, runFrame);
    }
    else {
        // 地面靜止：站立圖
        frameCounter = 0;
        updateKirbySprite("stop", dir, 0);
    }

    // 5. 攝影機跟隨
    view->centerOn(kirby->x(), 540);
    //x() 是 QGraphicsPixmapItem（以及所有 Qt 圖形元件）內建的一個成員函數。
    //回傳該物件目前在「舞台（Scene）」上的 X 座標（水平位置）。
}



// ---------------------------------------------------------
// 按鈕模塊
//---------------------------------------------------------
//把簡單的
//      if (event->key() == Qt::Key_Left) vx = -7;
//      else if (event->key() == Qt::Key_Right) vx = 7;
//       換成複雜的衝刺判定
//
//加入按鈕防止彈跳，雙擊有衝刺功能
//
//
// =========================================================
void MainWindow::keyPressEvent(QKeyEvent *event) {
    // [關鍵新增] 防彈跳：如果是作業系統長按產生的連發訊號，直接忽略
    if (event->isAutoRepeat()) {
        return;
    }

    int key = event->key();

    // 1. 處理蹲下
    if (key == Qt::Key_Down) {
        isDown = true;
        isDashing = false;
    }


    // 2. 處理跳躍與飛行
    // [新增] 處理大跳躍 (Z 鍵)，分割跳躍跟飛行邏輯
    else if (key == Qt::Key_Z) {
        // 只有在地面上，且沒有在吸氣的時候才能起跳
        if (kirby->y() >= 800 && !isInhaling) {
            vy = -15; // 給予較大的初始向上速度 (大跳躍，數值可依手感微調)
            isFlying = false; // 確保進入的是普通跳躍狀態
        }
    }
    // [修改] 處理飛行 (Up 鍵)
    else if (key == Qt::Key_Up) {
        // 無論在地面還是空中，只要沒在吸氣，按下「上」就強制切換成飛行模式
        if (!isInhaling) {
            isFlying = true;
            vy = -8;          // 拍翅膀給予的向上升力
            flapCounter = 8;  // 啟動拍翅膀動畫計時器，數字越大拍越快
        }
    }


    // 3. 處理右鍵
    else if (key == Qt::Key_Right) {
        if (doubleTapTimer->isActive() && lastReleasedKey == Qt::Key_Right) {
            isDashing = true;
            vx = DASH_SPEED;
            doubleTapTimer->stop();
            lastReleasedKey = -1;
        } else {
            if (!isDashing) vx = 7;
        }
    }
    // 4. 處理左鍵
    else if (key == Qt::Key_Left) {
        if (doubleTapTimer->isActive() && lastReleasedKey == Qt::Key_Left) {
            isDashing = true;
            vx = -DASH_SPEED;
            doubleTapTimer->stop();
            lastReleasedKey = -1;
        } else {
            if (!isDashing) vx = -7;
        }
    }

    //處理x吸氣
    else if (key == Qt::Key_X) { // 假設使用 Z 鍵當作攻擊/吸氣
        // 通常要在沒有蹲下、沒有飛行的狀態下才能吸氣
        if (!isDown && !isFlying) {
            isInhaling = true;
            isDashing = false; // 強制中斷衝刺
        }
    }


}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    // [關鍵新增] 防彈跳：過濾掉作業系統產生的「假放開」
    if (event->isAutoRepeat()) {
        return;
    }

    int key = event->key();

    // 處理放開蹲下
    if (key == Qt::Key_Down) {
        isDown = false;
    }
    // 處理放開左右鍵 (啟動雙擊計時)
    else if (key == Qt::Key_Left || key == Qt::Key_Right) {
        isDashing = false;
        vx = 0;

        // 啟動雙擊判定計時器
        lastReleasedKey = key;
        doubleTapTimer->start(DOUBLE_TAP_WINDOW);
    }

     // 處理放開 X鍵
    else if (key == Qt::Key_X) {
        isInhaling = false; // 放開按鍵停止吸氣
    }
}






// ---------------------------------------------------------
// dtor
// ---------------------------------------------------------
MainWindow::~MainWindow() {
    delete ui;
}
