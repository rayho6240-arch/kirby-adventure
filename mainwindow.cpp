#include "mainwindow.h"
#include "ui_mainwindow.h"

// 系統與除錯
#include <QKeyEvent>
#include <QDebug>

// 遊戲物件與畫面
#include <QGraphicsPixmapItem>
#include <QApplication>
#include "Block.h"
#include "Kirby.h"
#include "HUD.h"
#include "Sparky.h"

// =========================================================
// 1. 建構子與解構子 (初始化遊戲世界)
// =========================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // =========================================================
    // [修改] 將原本場景設定移至 loadStage1 ，這裡只留視窗大小與視角及計時器 
    // =========================================================

    // --- [1. 初始化場景 (Scene)] ---
    scene = new QGraphicsScene(this);

    // --- [2. 初始化視角 (View/Camera)] ---
    view = new QGraphicsView(scene, this);
    setCentralWidget(view);
    // 固定畫框大小，玩家看到的永遠是 1620x1080 的範圍
    view->setFixedSize(1620, 1080);



    // --- [8. 計時器與事件設定 (Timers & Events)] ---
    // 雙擊衝刺計時器 (只觸發一次)
    doubleTapTimer = new QTimer(this);
    doubleTapTimer->setSingleShot(true);
    connect(doubleTapTimer, &QTimer::timeout, this, &MainWindow::onDoubleTapTimerTimeout);

    // 遊戲主迴圈計時器 (約 60 FPS)
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::gameLoop);

    // 設定視窗可以接收鍵盤事件
    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocus();




    // 開始載入初始畫面
    loadStartMenu();
}


MainWindow::~MainWindow() {
    // ui 會自動刪除，scene 上面有 parent(this) 的物件也會被 Qt 回收
    delete ui;
}




// =========================================================
// 2. 遊戲主迴圈 (Game Loop)
// 每 16ms 執行一次，負責更新所有邏輯
// =========================================================
void MainWindow::gameLoop() {

    //---[0. 輸贏]---有問題~~~
    qDebug() << " 檢查卡比目前的血量：" << player->getCurrentHp();
    qDebug() << "卡比目前位置:" << "X:" << player->x() << "Y:" << player->y();
    if (currentState == GAMEOVER) return;

    // --- [1. 更新玩家狀態] ---
    if (player) {
        player->update();
        player->processInhale(enemyList); // 讓卡比偵測是否吸入前方的敵人
    }

    // --- [2. 更新敵人狀態] ---
    for (Enemy *e : enemyList) {
        e->update();
    }



    // --- [3. 物理碰撞偵測層] ---
    if (player) {
        for (Enemy *e : enemyList) {
            // @todo 這裡可以交給負責戰鬥機制的隊友擴充


            //  【新增這行防護罩】
            // 如果敵人已經判定死亡，或者「已經變成透明看不見了」，就直接跳過它，不檢查碰撞！
            // 目前敵人死掉都僅是看不見而已(顏色變透明，未來:直接把敵人刪掉。)
            if (e->getIsDead() || !e->isVisible()) {    //isVisible() 是 Qt 框架中 QGraphicsItem 類別內建的成員函數。
                continue;
            }

            if (player->collidesWithItem(e)) {
                // 取得卡比當前的動作狀態
                bool isInhaling = player->getInhaling(); // 是否正在吸氣
                bool isSpitting = player->getSpitting(); // 是否正在吐星星

                // 只有在卡比「沒有吸氣」且「沒有吐星星」的時候，才會受傷
                if (!isInhaling && !isSpitting) {
                    player->takeDamage(1);
                }
            }
        }
    }

    // --- [4. 更新星星子彈 (含邊界與銷毀判定)] ---
    for (int i = 0; i < bulletList.size(); ++i) {
        StarBullet *b = bulletList[i];
        b->update();

        // 如果星星飛太遠出界，或已被標記為不可見，則銷毀回收記憶體
        if (b->x() < 0 || b->x() > 4860 || !b->isVisible()) {
            scene->removeItem(b);
            bulletList.removeAt(i);
            delete b;
            i--; // @note List 數量減少，索引必須減 1，避免漏掉下一個元素
        }
    }

    // --- [4.5 子彈與敵人的碰撞偵測] ---
    for (int i = 0; i < bulletList.size(); ++i) {
        StarBullet *b = bulletList[i];
        if (!b->isVisible()) continue; // 如果子彈已經消失，跳過

        for (int j = 0; j < enemyList.size(); ++j) {
            Enemy *e = enemyList[j];

            // 只有「活著且看得見」的敵人才能被打中
            if (e->getIsDead() || !e->isVisible()) continue;

            // 檢查星星是否撞到敵人
            if (b->collidesWithItem(e)) {

                // 1. 讓敵人受傷或死亡 (呼叫敵人的死亡處理)
                e->setIsDead(true);
                e->setVisible(false); // 讓敵人先消失

                // 2. 讓星星子彈消失 (通常星星撞到東西會碎裂或消失)
                b->setVisible(false);

                // 3. 既然這顆星星已經撞到東西了，就不用再檢查其他敵人了
                break;
            }
        }
    }



    //  --- [5. 同步卡比的血量給 HUD] ---[新增]
    gameHUD->updateHealth(player->getCurrentHp(), player->getMaxHp(), player->getCurrentlives(), player->getMaxlives());

        // 讓 HUD 永遠跟著卡比走 (保持在視窗左上角)
    qreal uiX = player->x() - 350;
    if (uiX < 10) uiX = 10;
    gameHUD->setPos(uiX, 20);

        // 檢查死亡
    if (player->getCurrentHp() <= 0 && player->getCurrentlives() <= 1) {
        isGameOver = true;
        gameHUD->showGameOver();                 // 通知 HUD 顯示 Game Over
        gameHUD->setPos(player->x() - 100, 300); // 把字移到畫面中間
        loadGameOver();
    }
    else if(player->getCurrentHp() <= 0){
        if( currentState == STATE_STAGE1 ){
            player->setPos(400,100);
        }
        else if( currentState == STATE_STAGE2 ){
            player->setPos(400,100);
        }
        player->setCurrentlives();
        player->setCurrentHp();
    }

    // --- [6. 攝影機跟隨] ---
    // 等所有物件座標都算好後，最後移動視角
    if (player) {
        // qBound 限制攝影機不會拍到地圖外 (0 ~ 4860)
        // [修改] 將4860改為地圖實際寬度 width()，這樣切換到 stage2 時可直接套用
        qreal camX = qBound(512.0, player->x(), scene->width() - 512.0);
        view->centerOn(camX, 540);
    }
}

// =========================================================
// 3. 玩家輸入控制 (Input Handling)
// =========================================================

/**
 * @brief 雙擊計時器超時
 * @details 如果玩家沒有在指定時間內按第二下，重置紀錄，視為一般移動
 */
void MainWindow::onDoubleTapTimerTimeout() {
    lastReleasedKey = -1;
}

/**
 * @brief 按下按鍵事件
 */
void MainWindow::keyPressEvent(QKeyEvent *event) {
    // 防彈跳：忽略作業系統長按產生的連續假訊號
    if (event->isAutoRepeat()) return;

    int key = event->key();

    // ==========================================
    // [新增] 狀態 1：如果目前在主選單 (Start Menu)，則等待按下enter建
    // ==========================================
    if (currentState == STATE_MENU) {
        if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            currentState = STATE_STAGE1; // 更新狀態
            loadStage1();                // 載入第一關
        }
    }

    // ==========================================
    // [新增] 狀態 ：如果目前已經Game Over了，依據按上、下還有enter來決定之後的行動
    // ==========================================
    if (currentState == GAMEOVER) {
        if(key == Qt::Key_Return || key == Qt::Key_Enter){
            if(cont){
                loadStartMenu();
            }
            else{
                // [待解決]需要一個能夠關閉整個遊戲視窗的東西，下面這個也同樣會跳回start menu
                qApp->quit();
            }
        }
        else if(key == Qt::Key_Up){
            gameover->setPixmap(QPixmap(":/Project2_Dataset/Image/background/game_over_continue.png"));
            cont = true;
        }
        else if(key == Qt::Key_Down){
            gameover->setPixmap(QPixmap(":/Project2_Dataset/Image/background/game_over_quit.png"));
            cont = false;
        }
    }

    // ==========================================
    // [新增] 狀態 2：如果目前在 Stage1 並且在門的地方按下 Up ，則切換場景為 Stage2
    // ==========================================
    if (currentState == STATE_STAGE1) {
        if(player->x() < 4650 && player->x() > 4630 && player->getOnGround()){
            if(key == Qt::Key_Up){
                currentState = STATE_STAGE2;
                loadStage2();
                return;
            }
        }
    }


    // ==========================================
    // [新增包裝] 狀態 3：只有在遊戲關卡中，才執行卡比的動作按鍵
    // ==========================================
    if (currentState == STATE_STAGE1 || currentState == STATE_STAGE2) {
        if (!player) return; // 安全檢查：確保卡比存在才執行動作

        // 1. 蹲下
        if (key == Qt::Key_Down) {
            player->setDown(true);
            player->setDashing(false);
        }
        // 2. 跳躍
        else if (key == Qt::Key_Z) {
            player->jump();
        }
        // 3. 飛行
        else if (key == Qt::Key_Up) {
            // 注意：這裡如果你要實作碰到傳送門按上鍵進入 Stage 2，
            // 可以在這裡呼叫一個 player->checkPortal() 或是將邏輯寫在 player->fly() 裡判斷
            player->fly();
        }
        // 4. 往右移動與衝刺
        else if (key == Qt::Key_Right) {
            if (doubleTapTimer->isActive() && lastReleasedKey == Qt::Key_Right) {
                if (player-> getHasObjectInMouth() ) {
                    player->setHorizontalVelocity(7); // 吃東西時不能衝刺
                } else {
                    player->setDashing(true);
                    player->setHorizontalVelocity(12); // 衝刺速度
                }
                doubleTapTimer->stop();
                lastReleasedKey = -1;
            } else {
                if (!player->getVx()) player->setHorizontalVelocity(7); // 一般走路速度
            }
        }
        // 5. 往左移動與衝刺
        else if (key == Qt::Key_Left) {
            if (doubleTapTimer->isActive() && lastReleasedKey == Qt::Key_Left) {
                if (player-> getHasObjectInMouth() ) {
                    player->setHorizontalVelocity(-7); // 吃東西時不能衝刺
                } else {
                    player->setDashing(true);
                    player->setHorizontalVelocity(-12); // 衝刺速度
                }
                doubleTapTimer->stop();
                lastReleasedKey = -1;
            } else {
                if (!player->getVx()) player->setHorizontalVelocity(-7);
            }
        }
        // 6. 攻擊 (吸入/吐出)
        else if (key == Qt::Key_X) {
            player->handleAttack(); // 讓 Kirby 內部自行判斷目前狀態該執行哪個動作
        }
    }
}

/**
 * @brief 放開按鍵事件
 */
void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    // 防彈跳：過濾掉作業系統產生的「假放開」
    if (event->isAutoRepeat()) return;

    int key = event->key();

    // ==========================================
    // [新增包裝] 只有在遊戲關卡中，才處理放開按鍵的邏輯
    // ==========================================
    if (currentState == STATE_STAGE1 || currentState == STATE_STAGE2) {
        if (!player) return; // 安全檢查

        // 解除蹲下
        if (key == Qt::Key_Down) {
            player->setDown(false);
        }
        // 解除左右移動，並啟動雙擊計時器準備判定衝刺
        else if (key == Qt::Key_Left || key == Qt::Key_Right) {
            player->setDashing(false);
            player->setHorizontalVelocity(0);

            lastReleasedKey = key;
            doubleTapTimer->start(DOUBLE_TAP_WINDOW);
        }
        // 停止吸氣
        else if (key == Qt::Key_X) {
            player->stopInhaling();
        }
    }
}

// =========================================================
// [新增] 場景設定
// =========================================================
void MainWindow::loadStartMenu(){
    currentState = STATE_MENU; // 設定現在為初始場景
    scene->clear();
    bulletList.clear(); 
    enemyList.clear();
    scene->setSceneRect(0, 0, 1620, 1080);
    
    QGraphicsPixmapItem* title = new QGraphicsPixmapItem(QPixmap(":/Project2_Dataset/Image/background/start.png"));
    scene->addItem(title);
    
    // 停止遊戲迴圈，因為選單不需要更新物理運算
    timer->stop();
}


// 將原本mainwindow裡的程式碼移到loadStage1函式
void MainWindow::loadStage1(){
    timer->stop();

    // 1. 清空前一個場景 (Start Menu 或之前的關卡) 的所有物件
    // 注意：scene->clear() 會自動 delete 裡面的指標，避免記憶體外洩
    scene->clear(); 
    bulletList.clear(); 
    enemyList.clear();
    scene->setSceneRect(0, 0, 4860, 1080);

    // --- [3. 鋪設背景大圖 (Level Design)] ---
    // @note 將三張 1620 寬度的圖橫向拼接
    // [修改] 將背景圖片改為一整張地圖，加上一個藍色天空背景，**但畫質有點差**
    QPixmap bg(":/Project2_Dataset/Image/background/stage-1_1.png");
    QGraphicsPixmapItem* bg1 = new QGraphicsPixmapItem(bg);
    qreal scaleFactor = 4860.0 / bg.width();
    bg1->setScale(scaleFactor);
    bg1->setPos(0, 300);
    scene->addItem(bg1);

    QPixmap Bg(":/Project2_Dataset/Image/background/stage-1-bg.jpg");
    QGraphicsPixmapItem* bg2 = new QGraphicsPixmapItem(Bg);
    qreal ScaleFactor = 4860.0 / Bg.width();
    bg2->setScale(ScaleFactor);
    bg2->setPos(0, 0);
    scene->addItem(bg2);

    // 將背景圖層放到最後方，避免遮擋玩家與敵人
    bg1->setZValue(-10);
    bg2->setZValue(-20);

    // --- [4. 鋪設隱形地形碰撞 (Collisions)] ---
    // @note 這些是透明的 Block，用來阻擋卡比掉出地圖或穿牆
    // [修改] 將Block改為符合背景地圖的樣式
    Block* ground1 = new Block(0, 890, 2600, 400);
    scene->addItem(ground1);

    Block* wall1 = new Block(1360, 805, 130, 85);
    scene->addItem(wall1);

    Block* wall2 = new Block(2600, 805, 435, 485);
    scene->addItem(wall2);

    Block* wall3 = new Block(3375, 805, 1485, 485);
    scene->addItem(wall3);

    Block* wall4 = new Block(3835, 510, 125, 295);
    scene->addItem(wall4);

    Block* wall5 = new Block(3960, 660, 80, 145);
    scene->addItem(wall5);

    Block* ceil = new Block(0, -100, 4860, 100);
    scene->addItem(ceil);

    // [新增] 將方塊改成完全透明的，只留背景圖片
    ground1->setOpacity(0.0);
    wall1->setOpacity(0.0);
    wall2->setOpacity(0.0);
    wall3->setOpacity(0.0);
    wall4->setOpacity(0.0);
    wall5->setOpacity(0.0);
    
    // --- [5. 實體物件生成：玩家與敵人 (Entities)] ---
    // 玩家 (卡比)
    player = new Kirby(); //呼叫 Kirby.h 中的 ctor
    player->setPos(400, 100);
    scene->addItem(player);

    // 敵人 (Waddle Dee 軍隊)
    // @note 用迴圈批次生成敵人，並加入 enemyList 統一管理
    for (int i = 0; i < 3; ++i) {
        WaddleDee *newDee = new WaddleDee();
        newDee->setPos(800 + (i * 500), 500);
        scene->addItem(newDee);
        enemyList.append(newDee);
    }

    // --- [5. 誕生 HUD 並加入場景][新增] ---
    gameHUD = new HUD();
    scene->addItem(gameHUD);



    // --- [7. 訊號與槽連線 (Signals & Slots)] ---
    // 監聽卡比吐星星的訊號
    connect(player, &Kirby::starFired, this, [=](StarBullet* star){
        bulletList.append(star); // 只要卡比一噴，就加進清單
        qDebug() << "Captured a star! Total stars in list:" << bulletList.size();
    });
    timer->start(16);
}

// 未來將stage2的背景、人物程式直接寫在這裡
// 現在差地圖方塊設置以及背景優化
void MainWindow::loadStage2(){
    timer->stop();
    
    scene->clear(); 
    bulletList.clear(); 
    enemyList.clear();
    
    scene->setSceneRect(0, 0, 8100, 1080);

    QPixmap backg(":/Project2_Dataset/Image/background/stage2_merged_nowater.jpg");
    QGraphicsPixmapItem* bg2 = new QGraphicsPixmapItem(backg);
    qreal scaleFactor = 8100.0 / backg.width();
    bg2->setScale(scaleFactor);
    bg2->setPos(0, 400);
    scene->addItem(bg2);
    QPixmap Bg(":/Project2_Dataset/Image/background/stage-1-bg.jpg");
    QGraphicsPixmapItem* Bg2 = new QGraphicsPixmapItem(Bg);
    qreal ScaleFactor = 8100.0 / Bg.width();
    Bg2->setScale(ScaleFactor);
    Bg2->setPos(0, 0);
    scene->addItem(Bg2);

    bg2->setZValue(-10);
    Bg2->setZValue(-20);

    Block* ground1 = new Block(0, 890, 8100, 400);
    scene->addItem(ground1);

    // {新增}範例 B：建立一個三角形（斜坡）
    QPolygonF slope1;
    slope1 << QPointF(0, 100) << QPointF(130, 100) << QPointF(130, 0); // 三個頂點
    Block* ramp = new Block(1080,800,slope1);
    scene->addItem(ramp);



    player = new Kirby(); //呼叫 Kirby.h 中的 ctor
    player->setPos(400, 100);
    player->changeWidth(8100);
    scene->addItem(player);

    //新增敵人
    Sparky *spark = new Sparky(player);
    spark->setPos(500, 300); // 設定初始位置
    scene->addItem(spark);
    enemyList.append(spark); // 加進你的敵人陣列裡一起 update
    
    // --- [5. 誕生 HUD 並加入場景][新增] ---
    gameHUD = new HUD();
    scene->addItem(gameHUD);

    connect(player, &Kirby::starFired, this, [=](StarBullet* star){
        bulletList.append(star); // 只要卡比一噴，就加進清單
        qDebug() << "Captured a star! Total stars in list:" << bulletList.size();
    });
    
    timer->start(16);
}


void MainWindow::loadGameOver(){

    // 停止遊戲迴圈，因為選單不需要更新物理運算
    timer->stop();
    
    currentState = GAMEOVER;
    scene->clear(); 
    bulletList.clear(); 
    enemyList.clear();
    scene->setSceneRect(0,0,1620,1080);
    gameover = new QGraphicsPixmapItem(QPixmap(":/Project2_Dataset/Image/background/game_over_continue.png"));
    scene->addItem(gameover);

}