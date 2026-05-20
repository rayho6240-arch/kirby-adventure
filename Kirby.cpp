#include "Kirby.h"         // 引入卡比自己的標頭檔 (必須放在最前面)
#include "Block.h"         // 為了使用 qgraphicsitem_cast 辨識 Block 地形
#include "StarBullet.h"    // 吐出星星時需要用到

#include <QGraphicsScene>
#include <QList>
#include <QGraphicsItem>
#include <QtGlobal>        // 提供 qBound() 函數
#include <QDebug>

// =========================================================
// 1. 初始化與外部指令介面 (Initialization & Interface)
// =========================================================

/**
 * @brief Kirby 建構子 (出廠預設值)
 */
Kirby::Kirby() {
    // 1. ctor初始化卡比生命
    maxHp = 3;          // 假設卡比有 6 格血
    currentHp = maxHp;  // 出生時滿血

    maxlives = 3;
    currentlives = maxlives;

    isInvincible = false;
    invincibleTimer = 0;
    isSpitting = false;
    spitTimer = 0;

    // 2. 載入預設待機圖片
    QPixmap kirbyImg(":/Project2_Dataset/Image/Kirby_normal/kirby_stop_R.png");

    // 3. 賦予演員這張圖片 (此時尚未加入場景，由 MainWindow 負責加入)
    setPixmap(kirbyImg);
}




// --- 接收外部指令 (從 MainWindow 傳入) ---
void Kirby::setHorizontalVelocity(qreal v) { vx = v; }
void Kirby::setDashing(bool dashing){ isDashing = dashing; }
void Kirby::setDown(bool down) {
    this->isDown = down;
    qDebug() << "setDown called. isDown:" << isDown << " currentForm:" << (int)currentForm;
    // 小範圍重構：按下「下」時檢查變身
    if (isDown && currentForm == Form::SparkyFat) {
        currentForm = Form::Sparky;      // 正式變身
        hasObjectInMouth = false;        // 東西吞下去了，嘴巴空了
        this->setScale(1.0);
        qDebug() << "Kirby evolved to Sparky Form!";
        
        // 這裡可以播放一個「變身成功」的小特效
    }
}






void Kirby::stopInhaling(){ isInhaling = false; }

void Kirby::startInhaling() {
    // 蹲下或飛行時不能吸氣
    if (!isDown && !isFlying) isInhaling = true;
    isDashing = false;
}


// [修改] 將Mouthful狀態判斷加到jump、fly的動作判斷
void Kirby::jump() {
    // @note 以前是判斷 y() >= 800，現在完全交給 isOnGround 判斷，地形適應力更強！
    if (isOnGround && !isInhaling && !hasObjectInMouth) {
        vy = -15;           // 給予向上的初速度
        isOnGround = false; // 跳起的一瞬間，立刻解除踩地狀態
        isFlying = false;
    }
}

void Kirby::fly() {
    if (!isInhaling && !hasObjectInMouth) {
        isFlying = true;
        vy = -8;            // 拍打翅膀給予向上升力
        flapCounter = 8;    // 設定拍動動畫持續幀數
    }
}

// =========================================================
// 2. 核心遊戲物理與碰撞迴圈 (Core Physics & Update)
// =========================================================

/**
 * @brief 更新實體座標與碰撞判定 (每幀呼叫)
 * @note [設計理念] vx 是「玩家的意圖」，而 currentVx 是「物理引擎的最終裁決」。
 * 將 X 軸與 Y 軸的移動分開計算，是解決「卡在牆壁震盪」或「掉入懸崖」的關鍵技巧。
 */
void Kirby::update() {
    // --- 0. 定義固定物理碰撞箱大小 (Hitbox) ---
    const qreal KIRBY_PHYSICAL_HEIGHT = 120;
    //const qreal KIRBY_PHYSICAL_WIDTH = 120;   // 建議設為固定值，避免因切換動畫圖片大小不同而卡牆
    const qreal SAFETY_MARGIN  = 2;          // 安全間距，防止退回時剛好黏在牆壁邊緣 //目前只能通靈出這個數字，normal我碰撞振動問題成功修復，但是fat 會黏住

    // 【修正點】：不要寫死 const，根據卡比目前的狀態決定他有多胖！
    // 這裡的 isFlying 或是 isFat 請替換成你程式裡實際控制胖卡比的布林值
    qreal currentPhysicalWidth;
    qreal currentPhysicalHeight ;

    if (isFlying || isSpitting || currentForm == Form::SparkyFat) { // 將 SparkyFat 加入判斷：變胖時使用大尺寸 Hitbox
        currentPhysicalWidth = 180; 
        currentPhysicalHeight = 180; 
    }else {
        currentPhysicalWidth = 130; 
        currentPhysicalHeight = 130;
    }



    qreal oldY = y();

    // 套用重力 (飛行時重力減輕，產生漂浮感)
    if (isFlying) vy += (gravity * 0.65);
    else vy += gravity;

    // 如果蹲下或正在吸氣，強制停止水平移動
    qreal currentVx = (isDown || isInhaling) ? 0 : vx;


    //[關鍵修復] 計算圖片寬度與 Hitbox 的落差，確保 Hitbox 永遠在卡比的正中央
    qreal currentSpriteWidth = this->boundingRect().width();
    qreal offsetX = (currentSpriteWidth - currentPhysicalWidth) / 2.0;

    qreal currentSpriteHeight = this->boundingRect().height();
    // 【關鍵】：不用除以 2！因為所有的落差都在頭頂，腳底是切齊的。
    qreal offsetY = currentSpriteHeight - currentPhysicalHeight;





    // --- 1. 水平移動與碰撞 (X 軸) ---
    //------------------------------
    setX(x() + currentVx);

    // 世界邊界檢查 (防止走出地圖外)
    if (x() < 0) setX(0);
    // [修改] 將原本的邊界4860改為mapwidth，這樣到了stage2可透過 changeWidth(int width) 修改邊界
    if (x() > mapwidth - currentPhysicalWidth) setX(mapwidth - currentPhysicalWidth);

    // X 軸碰撞判定
    QList<QGraphicsItem *> collidingItemsX = scene()->collidingItems(this,Qt::IntersectsItemShape);
    for (QGraphicsItem * const &item : collidingItemsX) { //去看Block.h的註解
        Block *block = qgraphicsitem_cast<Block *>(item); //如果卡比碰到了「星星」或「特效」，因為它們不是 <Block>，cast 會回傳 nullptr，這樣卡比就不會把它們當作牆壁卡住了。
        if (block) {
            // 判定是否為「牆壁」：只要卡比的垂直範圍跟方塊有重疊，就是撞到牆
            bool isWall = (y() + KIRBY_PHYSICAL_HEIGHT > block->y() + 5) &&
                          (y() < block->y() + block->boundingRect().height() - 5);

            if (isWall) {
                // 1. 取得圖片真實寬度
                qreal actualWidth = this->boundingRect().width();

                // 2. 算出圖片與 120 物理箱之間的「置中偏差值」
                // 例如圖片是 130，那左右各超出了 5 像素 (offsetX = 5)
                qreal offsetX = (actualWidth - currentPhysicalWidth) / 2.0;

                // 3. 用真實寬度來抓中心點最準確
                qreal kirbyCenterX = this->x() + actualWidth / 2.0;
                qreal blockCenterX = block->x() + block->boundingRect().width() / 2.0;

                if (kirbyCenterX < blockCenterX) {
                    // --- 牆在右邊 ---
                    // 【修正點】：多扣除右邊超出去的 offsetX，確保圖片完全被推出牆外
                    setX(block->x() - currentPhysicalWidth - offsetX - SAFETY_MARGIN);

                    // 只有當你「正在往右走」時，才歸零速度
                    if (currentVx > 0) {
                        currentVx = 0;
                    }
                }
                else {
                    // --- 牆在左邊 ---
                    // 【修正點】：把左邊超出去的 offsetX 減掉，讓真正的 120 物理框去貼牆
                    setX(block->x() + block->boundingRect().width() - offsetX + SAFETY_MARGIN);

                    // 只有當你「正在往左走」時，才歸零速度
                    if (currentVx < 0)  currentVx = 0;
                }
            }//end if(isWall)
        }//end if(block)
    }//end for







    // --- 2. 垂直移動與碰撞 (Y 軸) ---
    setY(y() + vy);
    isOnGround = false; // 預設不在地上，等下如果踩到地板再設回 true

    QList<QGraphicsItem *> collidingItemsY = scene()->collidingItems(this,Qt::IntersectsItemShape);
    for (QGraphicsItem * const &item : collidingItemsY) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block) {
            // 下落碰撞 (踩到地板)
            if (vy >= 0 && (oldY + KIRBY_PHYSICAL_HEIGHT <= block->y() + 30)) {
                setY(block->y() - KIRBY_PHYSICAL_HEIGHT); // 腳貼齊地板
                vy = 0;             // 垂直速度歸零
                isOnGround = true;  // 標記為踩在地上
                break;
            }
            // 上升碰撞 (撞到天花板)
            else if (vy < 0 && (oldY >= block->y() + block->boundingRect().height() - 10)) {
                setY(block->y() + block->boundingRect().height()); // 頭頂貼齊天花板
                vy = 0;
            }
        }
    }

    // --- 3. 處理受傷後的無敵時間與閃爍特效[新增] ---
    if (isInvincible) {
        invincibleTimer--;

        // 利用餘數製造閃爍效果 (每 8 幀切換一次透明度)
        if (invincibleTimer % 8 < 4) {
            setOpacity(0.3); // 變半透明
        } else {
            setOpacity(1.0); // 恢復正常
        }

        // 時間到，解除無敵狀態
        if (invincibleTimer <= 0) {
            isInvincible = false;
            setOpacity(1.0); // 確保透明度恢復正常
        }
    }

    // --- 4. 處理吐星狀態的倒數 ---
    if (isSpitting) {
        spitTimer--;
        if (spitTimer <= 0) {
            isSpitting = false; // 時間到，解除吐星狀態
        }
    }


    // --- 5. 狀態補強與渲染 ---
    if (isOnGround) isFlying = false;         // 落地解除飛行狀態
    if (currentVx > 0) isFacingRight = true;  // 更新面朝方向
    else if (currentVx < 0) isFacingRight = false;
    if (hasObjectInMouth || isFlying)  isDashing = false; //變胖不可以衝刺

    updateSprite(); // 最後更新動畫幀
}

// =========================================================
// 3. 戰鬥與吸星系統 (Combat & Inhale Logic)
// =========================================================

/**
 * @brief 處理按下攻擊鍵(X)的總體邏輯
 */
void Kirby::handleAttack() {
    if (hasObjectInMouth) {
        spit();                             // [狀態 A]：嘴裡有東西 -> 噴射星星
    } else {
        startInhaling();                    // [狀態 B]：嘴裡沒東西 -> 開始吸氣
    }
}

/**
 * @brief 處理吸氣時對敵人的吸引力與吞食判定
 * @note 不使用單純的內建碰撞，而是手動計算距離 (dx)，給予敵人一個指向卡比的加速度 (vx)。
 */
void Kirby::processInhale(QList<Enemy*> &enemies) {
    // Sparky 形態時按 X 只是釋放閃電，不進行敵人吸引
    if (currentForm == Form::Sparky) return;
    if (!isInhaling) return;

    qreal inhaleRange = 300;  // 吸氣有效長度
    qreal inhaleHeight = 100; // 吸氣有效高度

    // 1. 定義偵測矩形 (Hitbox)
    QRectF inhaleRect;
    if (isFacingRight) {
        // 面朝右：起始點稍微往回縮一點 (x+10)，確保涵蓋嘴巴正前方
        inhaleRect = QRectF(x() + 10, y() - 20, inhaleRange, inhaleHeight);
    } else {
        // 面朝左
        inhaleRect = QRectF(x() - inhaleRange + 30, y() - 20, inhaleRange, inhaleHeight);
    }

    // 2. 遍歷畫面上的敵人
    for (Enemy *e : enemies) {
        if (!e->isVisible()) continue;

        if (inhaleRect.contains(e->pos())) {
            // dx: 敵人座標 與 卡比座標 的相對水平距離
            qreal dx = e->x() - x();

            // --- 吸引力物理 ---
            if (dx > 0) {
                e->vx = -6.0; // 敵人在右側，往左吸
                e->setIsBeingInhaled(true);
            }
            else{
                e->vx = 6.0;         // 敵人在左側，往右吸
                e->setIsBeingInhaled(true);
            } 
            

            // --- 吞食判定 (考慮面朝方向補償) ---
            bool shouldSwallow = false;

            if (isFacingRight) {
                // 面朝右：dx 介於 5 到 80 之間視為重疊吞入
                if (dx > 5 && dx < 80) shouldSwallow = true;
            } else {
                // 面朝左：利用絕對值判斷
                if (-dx > 5 && -dx < 40) shouldSwallow = true;
            }

            if (shouldSwallow) {
                // 1. 處理敵人死亡
                e->setVisible(false);
                e->setIsDead(true);

                // 2. 更新卡比狀態
                // 假設你的 Enemy 類別有一個 getType() 函數
                if (e->getEnemyType() == "Sparky") {
                    currentForm = Form::SparkyFat; // 進入準備變身狀態
                } else {
                    currentForm = Form::Normal;
                }

                setFullStatus(true); // 這會設定 hasObjectInMouth = true
                isInhaling = false;
                break; 
            }
        }
    }
}

/**
 * @brief 吐出星星攻擊
 */
void Kirby::spit() {
    qDebug() << "--- Spit Attempted ---";

    if (!hasObjectInMouth) {
        qDebug() << "Failed: Mouth is empty";
        return;
    }

    // 實體化星星子彈
    StarBullet *star = new StarBullet(x(), y()+30, isFacingRight);
    if (scene()) {
        scene()->addItem(star);
        emit starFired(star);          // 觸發訊號，通知 MainWindow 把星星加入更新清單
        qDebug() << "Signal Emitted!";
    }

    // 恢復正常狀態
    setFullStatus(false);
    isInhaling = false;
}

// =========================================================
// 4. 狀態切換模組 (State Transitions)
// =========================================================

/**
 * @brief 成功吞掉或吐出物件的那一刻，切換卡比的外觀狀態
 */
void Kirby::setFullStatus(bool full) {
    hasObjectInMouth = full;

    // [更改] 將變胖動畫加到下面的動畫渲染系統
    //if (full) {
        // TODO: 之後可以換成變胖的圖片 setPixmap(QPixmap(":/res/kirby_full.png"));
       // setScale(1.5);  // 先暫時放大 1.5 倍來 debug，一眼就看出吸到了
    //} else {
        // 恢復原狀
       // setScale(1.0);
    //}
}

void Kirby::discardAbility() {
    currentForm = Form::Normal;
    hasObjectInMouth = false;
    isInhaling = false;
    isSpitting = false;
    isFlying = false;
    isDashing = false;
    isDown = false;
    setScale(1.0);
}





// =========================================================
// 6. 生命&扣血系統[新增]
// =========================================================

void Kirby::takeDamage(int damage) {
    // 1. 如果正在無敵狀態，直接免疫這次傷害
    if (isInvincible) return;

    // 2. 扣血
    currentHp -= damage;
    qDebug() << "kirby wound ,HP remain：" << currentHp << "/" << maxHp;

    // 3. 死亡或觸發無敵
    if (currentHp <= 0 && currentlives > 0) {
        currentHp = 0;
        qDebug() << "kitby dead！";
        // TODO: 未來這裡要觸發卡比死亡動畫與遊戲結束邏輯
    } else {
        // 觸發無敵狀態 (假設 60 FPS，設定 60 幀約為 1 秒的無敵時間)
        isInvincible = true;
        invincibleTimer = 120;
    }
}

bool Kirby:: getInhaling(){
    return isInhaling;
}
bool Kirby:: getSpitting(){
    return isSpitting;
}

    // [新增] 得到是否在地上的資訊
bool Kirby::getOnGround(){
    return isOnGround;
}
    // [新增] 改變卡比邊界檢查的範圍
void Kirby::changeWidth(int width){
    mapwidth = width;
}



// =========================================================
// 6. 動畫渲染系統 (Animation & Rendering)
// =========================================================
void Kirby::updateSprite() {
    QString dir = isFacingRight ? "R" : "L";
    QString action;
    int frame = 0;

    // --- [核心重構 1] 決定基礎路徑 ---
    // 根據目前的型態決定要去哪個資料夾抓圖
    QString folderPath = ":/Project2_Dataset/Image/Kirby_normal/";
    if (currentForm == Form::Sparky) {
        folderPath = ":/Project2_Dataset/Image/Kirby_spark/"; // 假設你的 Sparky 素材放在這
    }

    // --- [原有邏輯] 決定動作與幀數 (保持不變) ---
    // 1. 跳躍狀態
    if (!isOnGround && !isFlying && qAbs(vy) > 2.0) {
        action = "jump";
        frame = (vy > 0) ? 3 : 1;
    }
    // 2. 飛行狀態
    else if (isFlying) {
        action = "fly";
        frame = (flapCounter > 0) ? 2 : 1;
        if (flapCounter > 0) flapCounter--;
    }
    // [變胖] 吃東西狀態 + 靜止
    else if (hasObjectInMouth && vx == 0) {
        action = "inhale";
        frame = 0;
    }
    // [變胖] 吃東西狀態 + 跑步
    else if (hasObjectInMouth && vx != 0) {
        action = "inhale";
        frameCounter++;
        frame = (frameCounter / 10) % 2 + 1;
    }
    // 3. 吸氣狀態
    else if (isInhaling && currentForm != Form::Sparky ) {
        action = "attack";
        frame = 0;
    }
    else if (isInhaling && currentForm == Form::Sparky ) {
        action = "attack";
        frameCounter++;
        frame = (frameCounter / 10) % 2 + 1;
    }
    // 4. 地面蹲下
    else if (isDown && isOnGround) {
        action = "down";
        frame = 0;
    }
    // 5. 地面衝刺
    else if (isDashing && vx != 0 && isOnGround && !hasObjectInMouth ) {
        action = "run";
        frameCounter++;
        frame = (frameCounter / 7) % 3 + 5;
    }
    // 6. 地面一般走路
    else if (vx != 0 && isOnGround) {
        action = "run";
        frameCounter++;
        frame = (frameCounter / 10) % 3 + 1;
    }
    // 7. 靜止待機
    else {
        action = "stop";
        frameCounter = 0;
        frame = 0;
    }






    // --- [核心重構 2] 組合圖片路徑 (改用 folderPath) ---
    QString path;

    if (currentForm == Form::Sparky) {
        // Sparky 形態：符合 "Kirby_spark_action(frame)_dir.png"
        QString folder = ":/Project2_Dataset/Image/Kirby_spark/"; // 請確認資源檔路徑
        
        if (action == "jump") {
            // 素材中似乎沒有 jump，暫時用 stop 代替，或你有補圖的話改回 "jump"
            path = QString("%1Kirby_spark_stop_%2.png").arg(folder).arg(dir);
        }
        else if (action == "attack") {
            // 攻擊幀：Kirby_spark_attack(1).png (不分左右)
            int sparkAttackFrame = (frameCounter / 5) % 3 + 1; // 假設攻擊有3幀
            path = QString("%1Kirby_spark_attack(%2).png").arg(folder).arg(sparkAttackFrame);
        }
        else if (frame == 0) {
            // 單幀動作：Kirby_spark_stop_R.png / Kirby_spark_down_R.png
            path = QString("%1Kirby_spark_%2_%3.png").arg(folder).arg(action).arg(dir);
        }
        else {
            // 多幀動作：Kirby_spark_run(1)_R.png / Kirby_spark_fly(1)_R.png
            // 修正幀數：Sparky 的跑跟飛只有 2 幀，要做循環限制防止讀不到圖
            int sparkFrame = (frameCounter / 10) % 2 + 1; 
            path = QString("%1Kirby_spark_%2(%3)_%4.png").arg(folder).arg(action).arg(sparkFrame).arg(dir);
        }
    } 
    else {
        // Normal 形態：保持你原本的路徑規則 (kirby_run_1_R.png)
        QString folder = ":/Project2_Dataset/Image/Kirby_normal/";
        if (action == "jump") {
            path = QString("%1kirby_jump(%2).png").arg(folder).arg(frame);
        } else if (frame == 0) {
            path = QString("%1kirby_%2_%3.png").arg(folder).arg(action).arg(dir);
        } else {
            path = QString("%1kirby_%2_%3_%4.png").arg(folder).arg(action).arg(frame).arg(dir);
        }
    }
    // --- 載入圖片與特殊處理 (保持不變) ---
    QPixmap pix(path);
    if (!pix.isNull()) {
        if (action == "jump" && dir == "L") {
            QImage flippedImage = pix.toImage().mirrored(true, false);
            pix = QPixmap::fromImage(flippedImage);
            
        }



        // --- 關鍵修正：讓圖片縮放到適合物理框的高度 ---
        // 假設我們希望卡比的身體（不含帽子）大約是 130 像素高
        // 我們可以強制將圖檔等比例縮放到高度 = 140 (或你覺得合適的數值)
        pix = pix.scaledToHeight(140, Qt::SmoothTransformation);



        setPixmap(pix);

        // --- 【核心修正：腳底對齊】 ---
        // 假設你設定卡比的物理碰撞盒高度是 130
        qreal baseHeight = 130.0; 
        
        // 計算 Y 軸偏移：(碰撞盒高度 - 圖片實際像素高度)
        // 如果圖片高 180 (有帽子)，yOffset = 130 - 180 = -50 (圖片會往上提 50 像素)
        // 如果圖片高 120 (跑步)，yOffset = 130 - 120 = 10 (圖片會往下壓 10 像素)
        qreal yOffset = baseHeight - pix.height();

        // --- 【水平修正：左右置中】 ---
        // 假設碰撞盒寬度是 130
        qreal xOffset = (130.0 - pix.width()) / 2.0;

        // 套用偏移，這不會移動碰撞盒，只會移動視覺上的圖片
        setOffset(xOffset, yOffset);


    } else {
        // Debug 用：如果路徑出錯，至少知道是哪張圖沒讀到
        // qDebug() << "Failed to load image:" << path;
    }
}
