#include "mainwindow.h"
#include "ui_mainwindow.h"

// 系統與除錯
#include <QKeyEvent>
#include <QDebug>
#include <cmath>

// 幫助我們得知.exe檔的路徑位置
#include <QCoreApplication>

#include <QDir>

// 遊戲物件與畫面
#include <QGraphicsPixmapItem>
#include <QApplication>
#include "Block.h"
#include "Kirby.h"
#include "HUD.h"
#include "Sparky.h"
#include "FloatingPlatform.h"
#include "Item.h"
#include "Slope.h"
#include "WaddleDoo.h"

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

    playlist = new QMediaPlaylist(this);
    
    // 💡 依序加入你所有的音樂（順序很重要！）
    // Index 0：主選單音樂
    QString menuPath1 = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../kirby-adventure/bg_music/01_Main_Title.mp3");
    QString menuPath2 = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/bg_music/01_Main_Title.mp3");
    QString menuPath = QFile::exists(menuPath1) ? menuPath1 : menuPath2; // 嘗試兩個路徑，哪個存在就用哪個
    // 🎯 加上這行，在下方的 Application Output 視窗看它印出什麼
    qDebug() << "真正的音樂路徑是：" << menuPath;
    playlist->addMedia(QUrl::fromLocalFile(menuPath));
    
    //Index 1：遊戲關卡背景音樂
    QString stagePath1 = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../kirby-adventure/bg_music/91_Vegetable_Valley_(No Intro).mp3");
    QString stagePath2 = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/bg_music/91_Vegetable_Valley_(No Intro).mp3");
    QString stagePath = QFile::exists(stagePath1) ? stagePath1 : stagePath2; // 嘗試兩個路徑，哪個存在就用哪個
    // 🎯 加上這行，在下方的 Application Output 視窗看它印出什麼// 🎯 加上這行，在下方的 Application Output 視窗看它印出什麼
    qDebug() << "真正的音樂路徑是：" << stagePath;
    playlist->addMedia(QUrl::fromLocalFile(stagePath));
    
    // Index 2：結局動畫音樂
    QString finishPath1 = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../kirby-adventure/bg_music/41_Goal.mp3");
    QString finishPath2 = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/bg_music/41_Goal.mp3");
    QString finishPath3 = QFile::exists(finishPath1) ? finishPath1 : finishPath2; // 嘗試兩個路徑，哪個存在就用哪個
    // 🎯 加上這行，在下方的 Application Output 視窗看它印出什麼
    qDebug() << "真正的音樂路徑是：" << finishPath3;
    playlist->addMedia(QUrl::fromLocalFile(finishPath3));
    

    // 設定循環模式
    playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);

    bgmPlayer = new QMediaPlayer(this);
    bgmPlayer->setPlaylist(playlist);
    bgmPlayer->setVolume(50);




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

        if ((currentState == STATE_STAGE1 || currentState == STATE_STAGE2) && player->y() > 1400) {
            qDebug() << "Kirby fell into abyss";
            if(player->getCurrentlives() > 1){
                player->minusCurrentlives(); // 直接扣光血量
            } else {
                player->setCurrentHp(0); // 扣最後一滴血，觸發死亡
            }
            player->setCurrentForm(Kirby::Form::Normal);
            if (!(player->getCurrentHp() <= 0 && player->getCurrentlives() <= 1)) {
                player->respawnAt(400, 100);
            }
        }
    }

    // --- [2. 更新敵人狀態] ---
    for (Enemy *e : enemyList) {
        e->update();
    }

    for (QGraphicsItem *sceneItem : scene->items()) {
        Item *item = dynamic_cast<Item *>(sceneItem);
        if (item && !item->isConsumed()) {
            item->update();
        }
    }

    if (boss && !boss->isDead()) {
        boss->update();
        QPointF bombPos;
        double bombVx = 0.0;
        double bombVy = 0.0;
        if (boss->consumeBombSpawnRequest(bombPos, bombVx, bombVy)) {
            Bomb *bomb = new Bomb(bombPos.x(), bombPos.y(), bombVx, bombVy, bossGroundY, player);
            scene->addItem(bomb);
            bombList.append(bomb);
            qDebug() << "Bomb spawned at" << bombPos << "vx =" << bombVx << "vy =" << bombVy;
        }
    }

    if (player) {
        const double inhalePullSpeed = 10.0;
        const double inhaleCompleteDistance = 35.0;

        for (int i = 0; i < bombList.size(); ++i) {
            Bomb *bomb = bombList[i];

            if (bomb->isDead() || bomb->isExploding()) {
                continue;
            }

            if (bomb->isBeingInhaled()) {
                QPointF mouthPos = player->getMouthScenePos();
                QPointF delta = mouthPos - bomb->sceneBoundingRect().center();
                double distance = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());

                bomb->moveToward(mouthPos, inhalePullSpeed);

                if (distance <= inhaleCompleteDistance) {
                    qDebug() << "Bomb inhale completed";
                    player->inhaleBomb();
                    bomb->markInhaled();
                    scene->removeItem(bomb);
                    bombList.removeAt(i);
                    delete bomb;
                    i--;
                }
                continue;
            }

            if (player->canInhaleBomb(bomb)) {
                bomb->startInhale();
                continue;
            }
        }
    }

    for (int i = 0; i < bombList.size(); ++i) {
        Bomb *bomb = bombList[i];
        bomb->update();

        if (bomb->isDead()) {
            scene->removeItem(bomb);
            bombList.removeAt(i);
            delete bomb;
            i--;
        }
    }

    for (int i = 0; i < bombStarList.size(); ++i) {
        BombStar *bombStar = bombStarList[i];
        bombStar->update();

        if (!bombStar->isExploding() && boss && !boss->isDead() && bombStar->collidesWithItem(boss)) {
            qDebug() << "BombStar hit Boss";
            boss->takeDamage(1);
            bombStar->startExplosion();
        }

        if (bombStar->isDead()) {
            scene->removeItem(bombStar);
            bombStarList.removeAt(i);
            delete bombStar;
            i--;
        }
    }



    // --- [3. 物理碰撞偵測層] ---
    if (player) {
        for (Enemy *e : enemyList) {
            // @todo 這裡可以交給負責戰鬥機制的隊友擴充


            //  【新增這行防護罩】
            // 如果敵人已經判定死亡，或者「已經變成透明看不見了」，就直接跳過它，不檢查碰撞！
            if (e->getIsDead() || !e->isVisible()) {    //isVisible() 是 Qt 框架中 QGraphicsItem 類別內建的成員函數。
                continue;
            }

            if (player->collidesWithItem(e)) {
                // 取得卡比當前的動作狀態
                bool isInhaling = player->getInhaling(); // 是否正在吸氣
                bool isSpitting = player->getSpitting(); // 是否正在吐星星
                bool isSparkyElectric = player->isSparkyElectricAttack();

                // Sparky 電擊狀態下，碰到敵人瞬間死亡
                if (isSparkyElectric) {
                    e->setIsDead(true);
                    e->setVisible(false);
                    continue;
                }

                // 只有在卡比「沒有吸氣」且「沒有吐星星」的時候，才會受傷
                if (!isInhaling && !isSpitting) {
                    player->takeDamage(1);
                }
            }
        }
    }

    // --- [4. 更新星星子彈 (含邊界與銷毀判定)] ---
    if (player && boss && !boss->isDead() && boss->isVisible()) {
        if (player->collidesWithItem(boss)) {
            player->takeDamage(1);
        }
    }

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

                // 1. 讓敵人瞬間死亡
                e->setIsDead(true);
                e->setVisible(false);

                // 2. 讓星星子彈消失 (通常星星撞到東西會碎裂或消失)
                b->setVisible(false);

                // 3. 既然這顆星星已經撞到東西了，就不用再檢查其他敵人了
                break;
            }
        }
    }



    //  --- [5. 同步卡比的血量給 HUD] ---[新增]
    gameHUD->updateHealth(player->getCurrentHp(), player->getMaxHp(), player->getCurrentlives(), player->getMaxlives());

    // 🎯 1. 告訴 Qt：我要視窗內左上角 (20, 20) 的地方。請幫我換算出場景的絕對座標！
    // (請把下面的 view 換成你程式裡 QGraphicsView 的變數名稱，例如 ui->graphicsView)
    QPointF hudScenePos = view->mapToScene(500, 930); 

    // 🎯 2. 直接把 HUD 貼過去，不論卡比怎麼飛、鏡頭怎麼晃，它在螢幕上的位置動都不會動！
    gameHUD->setPos(hudScenePos);

        // 檢查死亡
    if (player->getCurrentHp() <= 0 && player->getCurrentlives() <= 1) {
        isGameOver = true;
        gameHUD->showGameOver();                 // 通知 HUD 顯示 Game Over
        gameHUD->setPos(player->x() - 100, 300); // 把字移到畫面中間
        loadGameOver();
    }
    else if(player->getCurrentHp() <= 0){
        if( currentState == STATE_STAGE1 ){
            player->respawnAt(400,100);
        }
        else if( currentState == STATE_STAGE2 ){
            player->respawnAt(400,100);
        }
        player->minusCurrentlives();
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
            loadStage1();                // 載入第一關////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
        if(player->x() < 4680 && player->x() > 4600 && player->getOnGround()){
            if(key == Qt::Key_Up){
                currentState = STATE_STAGE2;
                loadStage2();
                return;
            }
        }
    }

    // ==========================================
    // [新增] 狀態 ：如果目前在 Stage2 並且在門的x位置的地方按下 Up ，則切換場景為 Finish
    // ==========================================
    if (currentState == STATE_STAGE2) {
        if(player->x() < 7900 && player->x() > 7800 && player->getOnGround() && player->y() < 600){
            if(key == Qt::Key_Up){
                currentState = STATE_STAGE3;
                loadStage3();
                return;
                
                /*currentState = STATE_FINISH;
                // 計算當前剩餘總血量
                remain_Hp = player->getCurrentHp() + player->getCurrentlives() * 3 - 3;
                loadFinish();
                return;
                */
            }
        }
    }

    if (currentState == STATE_STAGE3) {
        if(player->x() < 7800 && player->x() > 7600 && player->getOnGround()){
            if(key == Qt::Key_Up){
                currentState = STATE_STAGE4;
                loadStage4();
                return;
            }
        }
    }

    if (currentState == STATE_STAGE4) {
        if(player->x() < 7300 && player->x() > 7000 && player->getOnGround()){
            if(key == Qt::Key_Up){
                currentState = STATE_BOSS;
                loadBoss();
                return;
            }
        }
    }


    // ==========================================
    // [新增包裝] 狀態 3：只有在遊戲關卡中，才執行卡比的動作按鍵
    // ==========================================
    if (currentState == STATE_STAGE1 || currentState == STATE_STAGE2 || currentState == STATE_STAGE3 || currentState == STATE_STAGE4 || currentState == STATE_BOSS) {
        if (!player) return; // 安全檢查：確保卡比存在才執行動作

        // 💡 情況 A：選單目前是開啟狀態 -> 鍵盤全權控制選單操作
        if (isMenuOpen) {
            switch (key) {
                case Qt::Key_Left:
                    player->abilityMenu->moveLeft();
                    break;
                    
                case Qt::Key_Right:
                    player->abilityMenu->moveRight();
                    break;
                    
                case Qt::Key_Return: // 大鍵盤 Enter
                case Qt::Key_Enter:  // 數字鍵盤 Enter
                    {
                        AbilityType chosen = player->abilityMenu->getSelectedAbility();
                        
                        // 🔍 關鍵攔截：如果選到「未解鎖」的能力，直接無視這次 Enter
                        if (chosen == AbilityType::Beam && !player->abilityMenu->getUnlocked(1)) break;
                        if (chosen == AbilityType::Fire && !player->abilityMenu->getUnlocked(2)) break;
                        if (chosen == AbilityType::Spark && !player->abilityMenu->getUnlocked(3)) break;
                        
                        // 🟢 通過驗證，開始改變卡比型態（請對照你實際的 Form 變數）
                        if (chosen == AbilityType::Normal) {
                            player->setCurrentForm(Kirby::Form::Normal);
                        } else if (chosen == AbilityType::Beam && player->abilityMenu->getUnlocked(1)) {
                            player->setCurrentForm(Kirby::Form::BeamForm);
                        } else if (chosen == AbilityType::Fire && player->abilityMenu->getUnlocked(2)) {
                            player->setCurrentForm(Kirby::Form::FireForm);
                        } else if (chosen == AbilityType::Spark && player->abilityMenu->getUnlocked(3)) {
                            player->setCurrentForm(Kirby::Form::Sparky);
                        }
                        
                        // 關閉選單，解凍遊戲
                        isMenuOpen = false;
                        player->abilityMenu->setVisible(false);
                        timer->start(); // 🟢 遊戲物理恢復運作
                    }
                    break;
                case Qt::Key_V:
                    {
                        AbilityType chosen = player->abilityMenu->getSelectedAbility();
                        if (chosen == AbilityType::Normal) {
                            break;
                        } else if (chosen == AbilityType::Beam && player->abilityMenu->getUnlocked(1)) {
                            player->abilityMenu->setUnlocked(1, false); // 取消解鎖，讓它變成灰色
                        } else if (chosen == AbilityType::Fire && player->abilityMenu->getUnlocked(2)) {
                            player->abilityMenu->setUnlocked(2, false); // 取消解鎖，讓它變成灰色
                        } else if (chosen == AbilityType::Spark && player->abilityMenu->getUnlocked(3)) {
                            player->abilityMenu->setUnlocked(3, false); // 取消解鎖，讓它變成灰色
                        }
                    }    
                    break;

                case Qt::Key_Q: // 選單開啟時再按一次 Q，代表直接取消
                    isMenuOpen = false;
                    player->abilityMenu->setVisible(false);
                    timer->start(); // 🟢 遊戲解凍
                    break;
                    
                default:
                    // 當選單開啟時，無視其他按鍵（防止卡比在背景偷偷走路或跳躍）
                    break;
            }
            return; // 🛑 核心：直接中斷，不讓事件傳給下方原本的卡比控制
        }

        // 💡 情況 B：選單目前是關閉狀態 -> 正常遊玩中按下 Q 鍵
        if (key == Qt::Key_Q) {
            isMenuOpen = true;
            timer->stop(); // 🛑 關鍵：主計時器停止，全遊戲畫面與物理凍結！

            if (view) {
                // 1. 取得 View（視窗）目前的寬高中心點（視窗座標）
                QPoint viewCenter(view->width() / 2, view->height() / 2);
                
                // 2. 將這個視窗中央點，轉換成地圖（Scene）中的實際座標
                QPointF sceneCenter = view->mapToScene(viewCenter);
                
                // 3. 計算選單的左上角位置（扣掉選單自身寬高的一半：500/2=250, 140/2=70）
                qreal menuX = sceneCenter.x() - (player->abilityMenu->rect().width() / 2);
                qreal menuY = sceneCenter.y() - (player->abilityMenu->rect().height() / 2);
                
                // 4. 移動選單到目前畫面的絕對正中央！
                player->abilityMenu->setPos(menuX, menuY);
            }
            
            player->abilityMenu->resetSelection(); // 游標重置到第一個項目
            player->abilityMenu->setVisible(true); // 顯示長方形能力選單
            return;
        }

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
            if (player->canAutoFlap()) {
                player->setUpPressed(true);
                if (player->getOnGround()) {
                    player->jump();
                } else if (!player->isNormal()) {
                    player->fly();
                }
            } else {
                player->fly();
            }
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
        // 7. 棄置能力 / 平台下落
        else if (key == Qt::Key_V) {
            if (player->getDown() && player->isOnFloatingPlatform()) {
                player->setPassThroughPlatform(true);
            } else {
                player->discardAbility();
            }
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
    if (currentState == STATE_STAGE1 || currentState == STATE_STAGE2 || currentState == STATE_STAGE3 || currentState == STATE_STAGE4 || currentState == STATE_BOSS) {
        if (!player) return; // 安全檢查

        // 解除蹲下
        if (key == Qt::Key_Down) {
            player->setDown(false);
        }
        else if (key == Qt::Key_Up) {
            player->setUpPressed(false);
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
    bombList.clear();
    bombStarList.clear();
    enemyList.clear();
    boss = nullptr;
    scene->setSceneRect(0, 0, 1620, 1080);
    
    QGraphicsPixmapItem* title = new QGraphicsPixmapItem(QPixmap(":/Project2_Dataset/Image/background/start.png"));
    scene->addItem(title);
    
    // 🎯 遊戲一開始，預設播放第 0 首（主選單音樂）
    playlist->setCurrentIndex(0);
    bgmPlayer->play();

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
    bombList.clear();
    bombStarList.clear();
    enemyList.clear();
    boss = nullptr;
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
    AbilityMenu *menu = new AbilityMenu();
    player->abilityMenu = menu;
    player->setPos(400, 100);
    scene->addItem(player);
    scene->addItem(player->abilityMenu);

    // 敵人 (Waddle Dee 軍隊)
    // @note 用迴圈批次生成敵人，並加入 enemyList 統一管理
    for (int i = 0; i < 3; ++i) {
        WaddleDee *newDee = new WaddleDee();
        newDee->setPos(800 + (i * 500), 500);
        scene->addItem(newDee);
        enemyList.append(newDee);
    }

    // 敵人 (WaddleDoo - 光束鞭攻擊)
    WaddleDoo *doo1 = new WaddleDoo(player);
    doo1->setPos(1600, 520);
    scene->addItem(doo1);
    enemyList.append(doo1);

    WaddleDoo *doo2 = new WaddleDoo(player);
    doo2->setPos(2350, 520);
    scene->addItem(doo2);
    enemyList.append(doo2);

    WaddleDoo *doo3 = new WaddleDoo(player);
    doo3->setPos(4600, 520);
    scene->addItem(doo3);
    enemyList.append(doo3);

    // 敵人 (Gordo - 原地待機敵人)
    Gordo *gordo = new Gordo();
    gordo->setPos(1200, 600);
    scene->addItem(gordo);
    enemyList.append(gordo);

    // 敵人 (HotHead - 巡邏與噴火敵人)
    HotHead *hothead = new HotHead(player);
    hothead->setPos(2000, 500);
    scene->addItem(hothead);
    enemyList.append(hothead);

    // --- [5. 誕生 HUD 並加入場景][新增] ---
    gameHUD = new HUD();
    scene->addItem(gameHUD);

    playlist->setCurrentIndex(1); 
    bgmPlayer->play(); // 切換後確保有在播放





    // --- [7. 訊號與槽連線 (Signals & Slots)] ---
    // 監聽卡比吐星星的訊號
    connect(player, &Kirby::starFired, this, [=](StarBullet* star){
        bulletList.append(star); // 只要卡比一噴，就加進清單
        qDebug() << "Captured a star! Total stars in list:" << bulletList.size();
    });
    connect(player, &Kirby::bombStarFired, this, [=](BombStar* bombStar){
        bombStarList.append(bombStar);
    });
    timer->start(16);
}

// 未來將stage2的背景、人物程式直接寫在這裡
// 現在差地圖方塊設置以及背景優化
void MainWindow::loadStage2(){
    timer->stop();
    
    //繼承stage1的血量
    c_Hp = player->getCurrentHp();
    c_lives = player->getCurrentlives();
    currentform = player->getCurrentForm();
    currentUnlocked = player->abilityMenu->getUnlockedStatus();


    scene->clear(); 
    bulletList.clear(); 
    bombList.clear();
    bombStarList.clear();
    enemyList.clear();
    boss = nullptr;
    
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

    QPixmap bag(":/Project2_Dataset/Image/background/stage2_augment.jpg");
    QGraphicsPixmapItem* bgs = new QGraphicsPixmapItem(bag);
    bgs->setScale(scaleFactor);
    qreal pos = 160*scaleFactor + 400;
    bgs->setPos(0, pos);
    scene->addItem(bgs);

    bg2->setZValue(-10);
    Bg2->setZValue(-20);

    Block* ground1 = new Block(0, 890, 2720, 400);
    scene->addItem(ground1);

    Block* ground2 = new Block(3180, 890, 4920, 400);
    scene->addItem(ground2);

    Block* ceil = new Block(0, -100, 8100, 100);
    scene->addItem(ceil);

    //stage 2階梯碰撞方塊
    Block* stairBlock1 = new Block(2090, 825, 188, 60);//~~~~~~~~~~~~~~~~~~
    scene->addItem(stairBlock1);
    Block* stairBlock2 = new Block(2155, 760, 126, 60);
    scene->addItem(stairBlock2);
    Block* stairBlock3 = new Block(2220, 700, 64, 60);
    scene->addItem(stairBlock3);


    // {新增}範例 B：建立一個三角形（斜坡）
    QPolygonF slope1;
    slope1 << QPointF(0, 150) << QPointF(150, 150) << QPointF(150, 0); // 三個頂點
    Slope* ramp = new Slope(1100, 750, slope1);
    scene->addItem(ramp);
    QPolygonF slope2;
    slope2 << QPointF(0, 150) << QPointF(280, 150) << QPointF(280, 0); // 三個頂點
    Slope* ramp1 = new Slope(1250, 600, slope2);
    scene->addItem(ramp1);
    QPolygonF slope3;
    slope3 << QPointF(0, 150) << QPointF(0, 0) << QPointF(280, 150); // 三個頂點
    Slope* ramp2 = new Slope(1530, 600, slope3);
    scene->addItem(ramp2);
    QPolygonF slope4;
    slope4 << QPointF(0, 150) << QPointF(0, 0) << QPointF(150, 150); // 三個頂點
    Slope* ramp3 = new Slope(1810, 750, slope4);
    scene->addItem(ramp3);
    QPolygonF slope5;
    slope5 << QPointF(0, 140) << QPointF(250, 140) << QPointF(250, 0); // 三個頂點
    Slope* ramp4 = new Slope(2410, 755, slope5);
    scene->addItem(ramp4);
    QPolygonF slope6;
    slope6 << QPointF(0, 215) << QPointF(0, 0) << QPointF(400, 215); // 三個頂點
    Slope* ramp5 = new Slope(3230, 680, slope6);
    scene->addItem(ramp5);
    QPolygonF slope7;
    slope7 << QPointF(0, 75) << QPointF(130, 75) << QPointF(130, 0); // 三個頂點
    Slope* ramp6 = new Slope(4850, 815, slope7);
    scene->addItem(ramp6);
    QPolygonF slope8;
    slope8 << QPointF(0, 65) << QPointF(140, 65) << QPointF(140, 0); // 三個頂點
    Slope* ramp7 = new Slope(5240, 750, slope8);
    scene->addItem(ramp7);
    QPolygonF slope9;
    slope9 << QPointF(0, 65) << QPointF(120, 65) << QPointF(120, 0); // 三個頂點
    Slope* ramp8 = new Slope(5640, 685, slope9);
    scene->addItem(ramp8);
    QPolygonF slope10;
    slope10 << QPointF(0, 0) << QPointF(0, 75) << QPointF(130, 75); // 三個頂點
    Slope* ramp9 = new Slope(6500, 685, slope10);
    scene->addItem(ramp9);
    QPolygonF slope11;
    slope11 << QPointF(0, 0) << QPointF(0, 130) << QPointF(260, 130); // 三個頂點
    Slope* ramp10 = new Slope(7020, 760, slope11);
    scene->addItem(ramp10);


    Block* wall1 = new Block(2660, 755, 60, 140);
    scene->addItem(wall1);
    Block* wall2 = new Block(3180, 680, 50, 215);
    scene->addItem(wall2);
    Block* wall3 = new Block(4980, 815, 885, 75);
    scene->addItem(wall3);
    Block* wall4 = new Block(5380, 750, 495, 65);
    scene->addItem(wall4);   
    Block* wall5 = new Block(5760, 685, 115, 65);
    scene->addItem(wall5); 
    Block* wall6 = new Block(5875, 840, 515, 50);
    scene->addItem(wall6); 
    Block* wall7 = new Block(6390, 685, 110, 75);
    scene->addItem(wall7); 
    Block* wall8 = new Block(6390, 760, 630, 130);
    scene->addItem(wall8); 
    Block* block1 = new Block(2720, 950, 60, 60);
    scene->addItem(block1);
    Block* block2 = new Block(3120, 950, 60, 60);
    scene->addItem(block2);


    // {新增} 範例 C：單向藍色平台（測試用）
    FloatingPlatform *Plat1 = new FloatingPlatform(355, 750, 190, 30);
    scene->addItem(Plat1);
    FloatingPlatform *Plat2 = new FloatingPlatform(610, 690, 190, 30);
    scene->addItem(Plat2);
    FloatingPlatform *Plat3 = new FloatingPlatform(867, 620, 190, 30);
    scene->addItem(Plat3);
    FloatingPlatform *Plat4 = new FloatingPlatform(4045, 690, 285, 30);
    scene->addItem(Plat4);

    FloatingPlatform *Plat5 = new FloatingPlatform(4790, 630, 320, 30);
    scene->addItem(Plat5);
    FloatingPlatform *Plat5_1 = new FloatingPlatform(5235, 560, 190, 30);
    scene->addItem(Plat5_1);
    FloatingPlatform *Plat6 = new FloatingPlatform(5750, 500, 127, 30);
    scene->addItem(Plat6);
    FloatingPlatform *Plat7 = new FloatingPlatform(6389, 500, 127, 30);
    scene->addItem(Plat7);
    FloatingPlatform *Plat8 = new FloatingPlatform(7045, 560, 260, 30);
    scene->addItem(Plat8);
    FloatingPlatform *Plat9 = new FloatingPlatform(7680, 750, 253, 30);
    scene->addItem(Plat9);
    FloatingPlatform *Plat10 = new FloatingPlatform(7870, 630, 190, 30);
    scene->addItem(Plat10);


    player = new Kirby(); //呼叫 Kirby.h 中的 ctor
    AbilityMenu *menu = new AbilityMenu();
    player->abilityMenu = menu;
    player->setPos(400, 100);
    player->changeWidth(8100);
    scene->addItem(player->abilityMenu);

    // 繼承stage1的血量
    player->setCurrentHp(c_Hp);
    player->setCurrentlives(c_lives);
    player->setCurrentForm(currentform);
    player->abilityMenu->setUnlockedStatus(currentUnlocked);

    scene->addItem(player);

    boss = new Boss(bossArenaLeft, bossArenaRight, bossGroundY, player);
    scene->addItem(boss);

    //新增敵人sparky
    for (int i = 0; i < 3; ++i) {
        Sparky *spark = new Sparky(player);
        spark->setPos(800 + (i * 500), 500);
        scene->addItem(spark);
        enemyList.append(spark);
    }

    // --- [新增] Stage2 道具生成 (只生成一次) ---
    if (!maximTomatoSpawned) {
        MaximTomato *tomato = new MaximTomato();
        tomato->setPos(1600, 830);
        scene->addItem(tomato);
        maximTomatoSpawned = true;
    }
    if (!oneUpSpawned) {
        OneUp *oneup = new OneUp();
        oneup->setPos(1700, 830);
        scene->addItem(oneup);
        oneUpSpawned = true;
    }
    
    // --- [5. 誕生 HUD 並加入場景][新增] ---
    gameHUD = new HUD();
    scene->addItem(gameHUD);

    connect(player, &Kirby::starFired, this, [=](StarBullet* star){
        bulletList.append(star); // 只要卡比一噴，就加進清單
        qDebug() << "Captured a star! Total stars in list:" << bulletList.size();
    });
    connect(player, &Kirby::bombStarFired, this, [=](BombStar* bombStar){
        bombStarList.append(bombStar);
    });
    
    timer->start(16);
}


void MainWindow::loadGameOver(){

    // 停止遊戲迴圈，因為選單不需要更新物理運算
    timer->stop();
    
    currentState = GAMEOVER;
    scene->clear(); 
    bulletList.clear(); 
    bombList.clear();
    bombStarList.clear();
    enemyList.clear();
    boss = nullptr;
    scene->setSceneRect(0,0,1620,1080);
    gameover = new QGraphicsPixmapItem(QPixmap(":/Project2_Dataset/Image/background/game_over_continue.png"));
    scene->addItem(gameover);

}



// [待解決] 原本結算畫面我想直接截遊戲的影片，但好像qt不支援我的影片編碼，需要再想其他辦法
// [完成] 現在我把影片切成幾百張圖片來運作，在kirby-advanture資料夾中創建了一個finish_animation資料夾，裡面有finish_1~7種結算動畫
void MainWindow::loadFinish(){
    // 1. 停止所有遊戲計時器
    timer->stop();

    // 2. 徹底清空場景
    scene->clear(); 
    bulletList.clear(); 
    bombList.clear();
    bombStarList.clear();
    enemyList.clear();
    boss = nullptr;

    // 🔴 【核心修改：定義邏輯尺寸】
    // 我們將視窗的邏輯尺寸嚴格定義為 1620x1080，
    // 所有的座標（0,0）到（1620,1080）都必須被我們掌握。
    scene->setSceneRect(0, 0, 1620, 1080);
    
    // 初始化動畫元件
    finish_frame = 0;
    finish_Item = new QGraphicsPixmapItem();
    finish_Item->setPos(0, 0); 
    scene->addItem(finish_Item);
    
    // 優雅地讓鏡頭對準這個邏輯場景的最中心
    view->centerOn(810, 540); // (1620/2, 1080/2)

    // 當當前總血量為1~2時顯示 finish_7，並把 remain_Hp 鎖定在合法範圍 3..9
    if (remain_Hp <= 2) {
        remain_Hp = 3;
    }
    remain_Hp = qBound(3, remain_Hp, 9);

    int animationDir = qBound(1, 10 - remain_Hp, 7);

    // 🔴 【核心修正 2】安全路徑安全網（本機測試、助教編譯通通都能抓到）
    QString exePath = QCoreApplication::applicationDirPath();
    QString path1 = QDir::cleanPath(exePath + QString("/finish_animation/finish_%1/%2.png").arg(animationDir).arg(finish_frame));
    QString path2 = QDir::cleanPath(exePath + QString("/../../kirby-adventure/finish_animation/finish_%1/%2.png").arg(animationDir).arg(finish_frame));
    QString finalPath = QFile::exists(path1) ? path1 : path2;
    QPixmap firstPic(finalPath);

    playlist->setCurrentIndex(2);
    bgmPlayer->play();

    if (!firstPic.isNull()) {
        finish_Item->setPixmap(firstPic.scaled(1620, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    } else {
        qDebug() << "finish load failed:" << finalPath << "animationDir=" << animationDir << "remain_Hp=" << remain_Hp;
    }

    // 啟動動畫切換圖片計時器
    finish_timer = new QTimer(this);
    connect(finish_timer, &QTimer::timeout, this, &MainWindow::finish_animation);
    finish_timer->start(10);
}

void MainWindow::finish_animation(){
    int animationDir = qBound(1, 10 - remain_Hp, 7);
    int animationIndex = animationDir - 1;
    int totalFrames = finish_total[animationIndex];

    if (finish_frame >= totalFrames) {
        finish_timer->stop();
        loadStartMenu();
        return;
    }

    QString exePath = QCoreApplication::applicationDirPath();
    QString path1 = QDir::cleanPath(exePath + QString("/finish_animation/finish_%1/%2.png").arg(animationDir).arg(finish_frame));
    QString path2 = QDir::cleanPath(exePath + QString("/../../kirby-adventure/finish_animation/finish_%1/%2.png").arg(animationDir).arg(finish_frame));
    QString finalPath = QFile::exists(path1) ? path1 : path2;

    QPixmap originalPic(finalPath);
    if (!originalPic.isNull()) {
        finish_Item->setPixmap(originalPic.scaled(1620, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    } else {
        qDebug() << "finish frame load failed:" << finalPath << "frame=" << finish_frame << "animationDir=" << animationDir << "total=" << totalFrames;
    }

    finish_frame++;
}

void MainWindow::loadStage3(){
    timer->stop();
    
    //繼承stage1的血量
    c_Hp = player->getCurrentHp();
    c_lives = player->getCurrentlives();
    currentform = player->getCurrentForm();
    currentUnlocked = player->abilityMenu->getUnlockedStatus();


    scene->clear(); 
    bulletList.clear(); 
    bombList.clear();
    bombStarList.clear();
    enemyList.clear();
    boss = nullptr;
    
    scene->setSceneRect(0, 0, 8100, 1080);

    QPixmap backg(":/Project2_Dataset/Image/background/stage3.jpg");
    QGraphicsPixmapItem* bg2 = new QGraphicsPixmapItem(backg);
    qreal scaleFactor = 8100.0 / backg.width();
    bg2->setScale(scaleFactor);
    bg2->setPos(0, 0);
    scene->addItem(bg2);

    bg2->setZValue(-10);

    Block* ground1 = new Block(0, 450, 2358, 400);
    scene->addItem(ground1);

    Block* ground2 = new Block(2351, 700, 1200, 320);
    scene->addItem(ground2);
    Block* ground3 = new Block(3550, 460, 770, 320);
    scene->addItem(ground3);
    Block* ground4 = new Block(4809, 960, 865, 320);
    scene->addItem(ground4);
    Block* ground5 = new Block(6630, 460, 1290, 230);
    scene->addItem(ground5);

    Block* top1 = new Block(137, -40, 40, 550);
    scene->addItem(top1);
    Block* top2 = new Block(150, 0, 150, 60);
    scene->addItem(top2);
    Block* top3 = new Block(457, 0, 150, 60);
    scene->addItem(top3);
    Block* top4 = new Block(600, 40 , 460, 150);
    scene->addItem(top4);
    Block* top5 = new Block(600, 40 , 460, 150);
    scene->addItem(top5);
    Block* top6 = new Block(1220, 40 , 200, 150);
    scene->addItem(top6);

    Block* lastground = new Block(7500, 700 , 240, 270);
    scene->addItem(lastground);

    FloatingPlatform *Plat1 = new FloatingPlatform(5848, 830, 255, 52);
    scene->addItem(Plat1);
    FloatingPlatform *Plat2 = new FloatingPlatform(6240, 580, 255, 52);
    scene->addItem(Plat2);

    QPolygonF slope1;
    slope1 << QPointF(0, 300) << QPointF(0, 0) << QPointF(280, 300); // 三個頂點
    Slope* ramp1 = new Slope(4300, 440, slope1);
    scene->addItem(ramp1);
    QPolygonF slope2;
    slope2 << QPointF(0, 120) << QPointF(0, 0) << QPointF(270, 120); // 三個頂點
    Slope* ramp2 = new Slope(4560, 720, slope2);
    scene->addItem(ramp2);
    Block* ground2_1 = new Block(4780, 840, 40, 600);
    scene->addItem(ground2_1);


    player = new Kirby(); //呼叫 Kirby.h 中的 ctor
    AbilityMenu *menu = new AbilityMenu();
    player->abilityMenu = menu;
    player->setPos(400, 100);
    player->changeWidth(8100);
    scene->addItem(player->abilityMenu);


    // 繼承stage2的血量
    player->setCurrentHp(c_Hp);
    player->setCurrentlives(c_lives);
    player->setCurrentForm(currentform);
    player->abilityMenu->setUnlockedStatus(currentUnlocked);

    scene->addItem(player);

    boss = new Boss(bossArenaLeft, bossArenaRight, bossGroundY, player);
    scene->addItem(boss);

    //新增敵人sparky
    for (int i = 0; i < 3; ++i) {
        Sparky *spark = new Sparky(player);
        spark->setPos(800 + (i * 500), 100);
        scene->addItem(spark);
        enemyList.append(spark);
    }

    // --- [新增] Stage2 道具生成 (只生成一次) ---
    if (!maximTomatoSpawned) {
        MaximTomato *tomato = new MaximTomato();
        tomato->setPos(1600, 100);
        scene->addItem(tomato);
        maximTomatoSpawned = true;
    }
    if (!oneUpSpawned) {
        OneUp *oneup = new OneUp();
        oneup->setPos(1700, 100);
        scene->addItem(oneup);
        oneUpSpawned = true;
    }
    
    // --- [5. 誕生 HUD 並加入場景][新增] ---
    gameHUD = new HUD();
    scene->addItem(gameHUD);

    connect(player, &Kirby::starFired, this, [=](StarBullet* star){
        bulletList.append(star); // 只要卡比一噴，就加進清單
        qDebug() << "Captured a star! Total stars in list:" << bulletList.size();
    });
    connect(player, &Kirby::bombStarFired, this, [=](BombStar* bombStar){
        bombStarList.append(bombStar);
    });
    
    timer->start(16);
}

void MainWindow::loadStage4(){
    timer->stop();
    
    //繼承stage1的血量
    c_Hp = player->getCurrentHp();
    c_lives = player->getCurrentlives();
    currentform = player->getCurrentForm();
    currentUnlocked = player->abilityMenu->getUnlockedStatus();

    scene->clear(); 
    bulletList.clear(); 
    bombList.clear();
    bombStarList.clear();
    enemyList.clear();
    boss = nullptr;
    
    scene->setSceneRect(0, 0, 8100, 1080);
    

    QPixmap backg(":/Project2_Dataset/Image/background/stage4.jpg");
    QGraphicsPixmapItem* bg2 = new QGraphicsPixmapItem(backg);
    qreal scaleFactor = 8100.0 / backg.width();
    bg2->setScale(scaleFactor);
    bg2->setPos(0, -100);
    scene->addItem(bg2);

    bg2->setZValue(-10);

    Block* ground0_1 = new Block(0, 890, 1069, 400);
    scene->addItem(ground0_1);
    Block* ground0_2 = new Block(2158, 890, 2722, 400);
    scene->addItem(ground0_2);
    Block* ground0_3 = new Block(5569, 890, 2531, 400);
    scene->addItem(ground0_3);
    Block* ground0_1_1 = new Block(1503, 990, 660, 60);
    scene->addItem(ground0_1_1);
    Block* ground0_2_1 = new Block(4830, 990, 721, 60);
    scene->addItem(ground0_2_1);

    Block* ground1 = new Block(1281, 620, 250, 350);
    scene->addItem(ground1);
    Block* ground2 = new Block(1129, 730, 160, 160);//左
    scene->addItem(ground2);
    Block* ground3 = new Block(2557, 730, 500, 160);//右
    scene->addItem(ground3);
    Block* ground4 = new Block(4551, 620, 220, 300);//右
    scene->addItem(ground4);
    Block* ground5 = new Block(4243, 760, 250, 240);//左
    scene->addItem(ground5);
    Block* ground6 = new Block(5050, 610, 350, 140);//福
    scene->addItem(ground6);
    Block* ground7 = new Block(5714, 610, 290, 270);//左
    scene->addItem(ground7);
    Block* ground8 = new Block(6000, 465, 205, 485);//右
    scene->addItem(ground8);
    Block* ground9 = new Block(7030, 620, 350, 310);
    scene->addItem(ground9);



    player = new Kirby(); //呼叫 Kirby.h 中的 ctor
    AbilityMenu *menu = new AbilityMenu();
    player->abilityMenu = menu;
    player->setPos(400, 100);
    player->changeWidth(8100);
    scene->addItem(player->abilityMenu);

    // 繼承stage3的血量
    player->setCurrentHp(c_Hp);
    player->setCurrentlives(c_lives);
    player->setCurrentForm(currentform);
    player->abilityMenu->setUnlockedStatus(currentUnlocked);

    scene->addItem(player);

    boss = new Boss(bossArenaLeft, bossArenaRight, bossGroundY, player);
    scene->addItem(boss);

    //新增敵人sparky
    for (int i = 0; i < 3; ++i) {
        Sparky *spark = new Sparky(player);
        spark->setPos(800 + (i * 500), 100);
        scene->addItem(spark);
        enemyList.append(spark);
    }

    // --- [新增] Stage2 道具生成 (只生成一次) ---
    if (!maximTomatoSpawned) {
        MaximTomato *tomato = new MaximTomato();
        tomato->setPos(1600, 100);
        scene->addItem(tomato);
        maximTomatoSpawned = true;
    }
    if (!oneUpSpawned) {
        OneUp *oneup = new OneUp();
        oneup->setPos(1700, 100);
        scene->addItem(oneup);
        oneUpSpawned = true;
    }
    
    // --- [5. 誕生 HUD 並加入場景][新增] ---
    gameHUD = new HUD();
    scene->addItem(gameHUD);

    connect(player, &Kirby::starFired, this, [=](StarBullet* star){
        bulletList.append(star); // 只要卡比一噴，就加進清單
        qDebug() << "Captured a star! Total stars in list:" << bulletList.size();
    });
    connect(player, &Kirby::bombStarFired, this, [=](BombStar* bombStar){
        bombStarList.append(bombStar);
    });
    
    timer->start(16);
}

void MainWindow::loadBoss(){
    timer->stop();
    
    //繼承stage1的血量
    c_Hp = player->getCurrentHp();
    c_lives = player->getCurrentlives();
    currentform = player->getCurrentForm();
    currentUnlocked = player->abilityMenu->getUnlockedStatus();

    scene->clear(); 
    bulletList.clear(); 
    bombList.clear();
    bombStarList.clear();
    enemyList.clear();
    boss = nullptr;
    
    scene->setSceneRect(0, 0, 4860, 1080);

    QPixmap backg(":/Project2_Dataset/Image/background/stage-boss.jpg");
    QGraphicsPixmapItem* bg2 = new QGraphicsPixmapItem(backg);
    qreal scaleFactor = 4860.0 / backg.width();
    bg2->setScale(scaleFactor);
    bg2->setPos(0, 100);
    scene->addItem(bg2);
    QPixmap Bg(":/Project2_Dataset/Image/background/stage-1-bg.jpg");
    QGraphicsPixmapItem* Bg2 = new QGraphicsPixmapItem(Bg);
    qreal ScaleFactor = 4860.0 / Bg.width();
    Bg2->setScale(ScaleFactor);
    Bg2->setPos(0, 0);
    scene->addItem(Bg2);

    bg2->setZValue(-10);
    Bg2->setZValue(-20);

    Block* ground1 = new Block(0, 990, 4860, 100);
    scene->addItem(ground1);
    Block* ground2 = new Block(1718, 780, 202, 330);
    scene->addItem(ground2);
    Block* ground3 = new Block(3690, 780, 202, 330);
    scene->addItem(ground3);

    Block* last1 = new Block(4416, 380, 272, 300);
    scene->addItem(last1);
    Block* last2 = new Block(4730, -10, 60, 1000);
    scene->addItem(last2);

    QPolygonF slope1;
    slope1 << QPointF(0, 232) << QPointF(0, 0) << QPointF(-350, 232); // 三個頂點
    Slope* ramp1 = new Slope(1710, 790, slope1);
    scene->addItem(ramp1);
    QPolygonF slope2;
    slope2 << QPointF(0, 282) << QPointF(0, 0) << QPointF(400, 282); // 三個頂點
    Slope* ramp2 = new Slope(3890, 790, slope2);
    scene->addItem(ramp2);
    QPolygonF slope3;
    slope3 << QPointF(0, 150) << QPointF(0, 0) << QPointF(-272, 150); // 三個頂點
    Slope* ramp3 = new Slope(4716, 230, slope3);
    scene->addItem(ramp3);




    player = new Kirby(); //呼叫 Kirby.h 中的 ctor
    AbilityMenu *menu = new AbilityMenu();
    player->abilityMenu = menu;
    player->setPos(400, 100);
    player->changeWidth(4860);
    scene->addItem(player->abilityMenu);

    // 繼承stage4的血量
    player->setCurrentHp(c_Hp);
    player->setCurrentlives(c_lives);
    player->setCurrentForm(currentform);
    player->abilityMenu->setUnlockedStatus(currentUnlocked);

    scene->addItem(player);
    boss = new Boss(bossArenaLeft, bossArenaRight, bossGroundY, player);
    scene->addItem(boss);

    //新增敵人sparky
    for (int i = 0; i < 3; ++i) {
        Sparky *spark = new Sparky(player);
        spark->setPos(800 + (i * 500), 100);
        scene->addItem(spark);
        enemyList.append(spark);
    }

    // --- [新增] Stage2 道具生成 (只生成一次) ---
    if (!maximTomatoSpawned) {
        MaximTomato *tomato = new MaximTomato();
        tomato->setPos(1600, 100);
        scene->addItem(tomato);
        maximTomatoSpawned = true;
    }
    if (!oneUpSpawned) {
        OneUp *oneup = new OneUp();
        oneup->setPos(1700, 100);
        scene->addItem(oneup);
        oneUpSpawned = true;
    }
    
    // --- [5. 誕生 HUD 並加入場景][新增] ---
    gameHUD = new HUD();
    scene->addItem(gameHUD);

    connect(player, &Kirby::starFired, this, [=](StarBullet* star){
        bulletList.append(star); // 只要卡比一噴，就加進清單
        qDebug() << "Captured a star! Total stars in list:" << bulletList.size();
    });
    connect(player, &Kirby::bombStarFired, this, [=](BombStar* bombStar){
        bombStarList.append(bombStar);
    });
    
    timer->start(16);
}
