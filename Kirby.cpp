//結構清晰：卡比怎麼跳、怎麼變胖都在 Kirby.cpp 裡。
//容易擴展：之後你要做敵人（Waddle Dee），只要再創一個 Enemy 類別，
//              並在 gameLoop 裡加上 enemy->update() 就好了。
//

//========================================================
#include "Kirby.h"         // 引入卡比自己的標頭檔 (必須放在最前面)
#include "Block.h"         // [關鍵新增] 因為我們要用 qgraphicsitem_cast 辨識 Block
#include "StarBullet.h"

#include <QGraphicsScene>  // 為了呼叫 scene()->collidingItems(this) 來獲取場景
#include <QList>           // 為了裝 collidingItems 回傳的碰撞清單
#include <QGraphicsItem>   // 為了讀取 QGraphicsItem 的指標
#include <QtGlobal>        // 為了使用 qBound() 函數 (通常 Qt 內建已包，但明確加上較好)



Kirby::Kirby() {
    // 1. 載入圖片資料
    QPixmap kirbyImg(":/Project2_Dataset/Image/Kirby_normal/kirby_stop_R.png");
    // 2. 創立演員，把圖片給他，還沒上台 (上台在 MainWindow 處理)
    setPixmap(kirbyImg);
}

// 接收外部指令的介面
void Kirby::setHorizontalVelocity(qreal v) { vx = v; }
void Kirby::setDashing(bool dashing) { isDashing = dashing; }
void Kirby::setDown(bool down) { isDown = down; }
void Kirby::startInhaling() { if (!isDown && !isFlying) isInhaling = true; isDashing = false; }
void Kirby::stopInhaling() { isInhaling = false; }

void Kirby::jump() {
    // [關鍵修改] 刪除 y() >= 800，完全交給 isOnGround 來判斷！
    if (isOnGround && !isInhaling) {
        vy = -15;
        isOnGround = false; // 跳起的一瞬間，地面狀態立刻解除
        isFlying = false;
    }
}

void Kirby::fly() {
    if (!isInhaling) {
        isFlying = true;
        vy = -8;
        flapCounter = 8;
    }
}






// ---------------------------------------------------------
// 核心遊戲迴圈 (從原本的 gameLoop 搬過來)
// ---------------------------------------------------------
//vx 是「玩家的意圖」，而 currentVx 是「物理引擎的最終裁決」
//vx 是private變數， currentVx 是區域變數，用來算nextX
//bug: 朝右撞牆，若突然折返，會穿過牆壁掉入懸崖
//bug: 朝左小跳，沒跳過牆，會震盪
//==============================================================
void Kirby::update() {
    // --- 0. 定義固定物理大小 (這是解決震動的關鍵) ---
    const qreal KIRBY_PHYSICAL_HEIGHT = 120;
    const qreal KIRBY_PHYSICAL_WIDTH = 120;   // 根據你的卡比圖片寬度調整，建議設為固定值
    const qreal SAFETY_MARGIN = 1.0;         // 安全間距，防止黏在牆上

    qreal oldY = y();
    if (isFlying) vy += (gravity * 0.65);
    else vy += gravity;
    qreal currentVx = (isDown || isInhaling) ? 0 : vx;

    // --- 1. 水平移動與碰撞 (X 軸) ---
    setX(x() + currentVx);

    // 世界邊界檢查
    if (x() < 0) setX(0);
    if (x() > 4860 - KIRBY_PHYSICAL_WIDTH) setX(4860 - KIRBY_PHYSICAL_WIDTH);

    QList<QGraphicsItem *> collidingItemsX = scene()->collidingItems(this);
    for (QGraphicsItem * const &item : collidingItemsX) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block) {
            // 判定是否為「牆壁」：只要卡比的垂直範圍跟方塊有重疊，就是牆壁
            bool isWall = (y() + KIRBY_PHYSICAL_HEIGHT > block->y() + 5) &&
                          (y() < block->y() + block->boundingRect().height() - 5);

            if (isWall) {
                if (currentVx > 0) {
                    // 往右撞：推回到方塊左緣，再多退後 1 像素
                    setX(block->x() - KIRBY_PHYSICAL_WIDTH - SAFETY_MARGIN);
                } else if (currentVx < 0) {
                    // 往左撞：推回到方塊右緣，再多前進 1 像素
                    setX(block->x() + block->boundingRect().width() + SAFETY_MARGIN);
                }
            }
        }
    }

    // --- 2. 垂直移動與碰撞 (Y 軸) ---
    setY(y() + vy);
    isOnGround = false;

    QList<QGraphicsItem *> collidingItemsY = scene()->collidingItems(this);
    for (QGraphicsItem * const &item : collidingItemsY) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block) {
            // 下落碰撞 (踩地)
            if (vy >= 0 && (oldY + KIRBY_PHYSICAL_HEIGHT <= block->y() + 30)) {
                setY(block->y() - KIRBY_PHYSICAL_HEIGHT);
                vy = 0;
                isOnGround = true;
                break;
            }
            // 上升碰撞 (撞天花板)
            else if (vy < 0 && (oldY >= block->y() + block->boundingRect().height() - 10)) {
                setY(block->y() + block->boundingRect().height());
                vy = 0;
            }
        }
    }

    // --- 3. 狀態補強 ---
    if (isOnGround) isFlying = false;
    if (currentVx > 0) isFacingRight = true;
    else if (currentVx < 0) isFacingRight = false;

    updateSprite();
}




// ---------------------------------------------------------
// 吸星大法
// ---------------------------------------------------------
//這裡我們不使用簡單的碰撞，而是手動計算距離，給予敵人一個指向卡比的加速度。
//
//
//============================================================
void Kirby::processInhale(QList<Enemy*> &enemies) {
    if (!isInhaling) return;

    qreal inhaleRange = 300;
    qreal inhaleHeight = 100;
    const qreal SWALLOW_DISTANCE = 35.0; // 統一吞掉判定距離

    // 1. 定義偵測矩形
    QRectF inhaleRect;
    if (isFacingRight) {
        // [修正]：面朝右時，偵測區起始點稍微往回縮一點，確保涵蓋嘴巴前方
        inhaleRect = QRectF(x() + 10, y() - 20, inhaleRange, inhaleHeight);
    } else {
        inhaleRect = QRectF(x() - inhaleRange + 30, y() - 20, inhaleRange, inhaleHeight);
    }

    // 2. 遍歷敵人
    for (Enemy *e : enemies) {
        if (!e->isVisible()) continue;

        if (inhaleRect.contains(e->pos())) {

            // [核心修正]：計算「真正的中心距離」
            // 我們假設卡比圖片寬度大約是 60，方塊寬度大約是 60
            // dx 代表敵人的幾何中心 與 卡比圖片 (x, y) 的相對關係
            qreal dx = e->x() - x();

            // --- 吸引力物理 --
            // 敵人在卡比右邊 (dx > 0)，往左吸 (-6.0)
            if (dx > 0) e->vx = -6.0;
            // 敵人在卡比左邊 (dx < 0)，往右吸 (6.0)
            else e->vx = 6.0;

            // --- [修正]：吞掉判定 (引入朝向補償) ---
            // 因為圖片左上角點的問題，朝向不同時，嘴巴的「幾何座標」相對於 x() 是不同的。
            bool shouldSwallow = false;

                //如果朝右還是覺得「太晚消失」（太重疊了），請把 dx 調大
            if (isFacingRight) {
                // 面朝右：Block 必須在 Kirby 的右邊 (dx > 0)
                // 且距離 x() 的位置要在合理範圍 (例如 Block.x 落在 Kirby.x + 10 到 + 40 之間)
                // 這裡我們取一個讓 Block 稍微重疊 Kirby 身體的值
                if (dx > 5 && dx < 80) {
                    shouldSwallow = true;
                }
            }
            else {
                // 面朝左：Block 必須在 Kirby 的左邊 (dx < 0)
                // Block.x 落在 Kirby.x - 30 到 0 之間
                // 我們利用 dx 已經是負值，所以這裡判斷 -dx (即絕對值)
                if (-dx > 5 && -dx < 40) {
                    shouldSwallow = true;
                }
            }

            if (shouldSwallow) {
                e->setVisible(false);
                // 標記死亡，這會讓 e->update() 裡的 return 觸發
                e->setIsDead(true);

                // 物理清零，確保它不會再移動
                e->vx = 0;
                e->vy = 0;

                // [重點]：卡比狀態切換
                setFullStatus(true);

                // 自動停止吸氣動作，避免一次吸入多個
                isInhaling = false;

                // [進階建議]：既然吸到了，就直接跳出迴圈，不要再掃描其他敵人
                break;
            }

        } //endif
    }//endfor
}//endprocessInhale




// ---------------------------------------------------------
// 成功吞掉敵人的那一刻，狀態接換。
// ---------------------------------------------------------
//
//
//
//============================================================
void Kirby::setFullStatus(bool full) {
    hasObjectInMouth = full;

    if (full) {
        // 切換成變胖的圖片 (現在可以先用 setScale 稍微放大來 debug)
        // setPixmap(QPixmap(":/res/kirby_full.png"));
        setScale(1.5); // 先變大 1.2 倍，一眼就看出吸到了
    } else {
        // 恢復原狀
        // setPixmap(QPixmap(":/res/kirby_normal.png"));
        setScale(1.0);
    }
}




void Kirby::handleAttack() {
    if (hasObjectInMouth) {
        // [狀態 A]：嘴裡有東西 -> 噴射星星
        spit();
    } else {
        // [狀態 B]：嘴裡沒東西 -> 開始吸氣
        startInhaling();
    }
}



void Kirby::spit() {
    if (!hasObjectInMouth) return;

    // 1. 生成星星子彈 (座標可以根據嘴巴位置調整)
    qreal spawnX = isFacingRight ? (x() + 40) : (x() - 40);
    StarBullet *star = new StarBullet(spawnX, y() + 10, isFacingRight);

    // 2. 將星星加入場景
    if (scene()) {
        scene()->addItem(star);
        // [重要提示]：如果你的 MainWindow 有 bulletList，
        // 這裡可以透過 Signal 發送出去，或是讓 MainWindow 在下一幀自動偵測 scene 裡的星星
    }

    // 3. 恢復身材狀態
    setFullStatus(false);

    // 4. 噴射完後確保吸氣狀態是關閉的
    isInhaling = false;
}











// ---------------------------------------------------------
// 萬用換圖函數：處理各種檔名規律
// ---------------------------------------------------------
//靠 gameLoop 裡的 「計時器」 加上 「取餘數（%）」 運算。
//第一個 .arg(action)：它會掃描字串，找到第一個出現的 %1，然後把 action 變數的內容（例如 "run"）塞進去。
//第二個 .arg(frame)：它會找 %2，把計算好的數字（例如 1）塞進去。
//第三個 .arg(dir)：它會找 %3，把方向（例如 "R"）塞進去。
//新增水平翻轉，最好還是透過改檔名跟截圖素材比較好，但就是紀錄一下有這種工具pix.toImage().mirrored(true, false);
//==============================================================
void Kirby::updateSprite() {
    QString dir = isFacingRight ? "R" : "L";
    QString action;
    int frame = 0;

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
    // 3. 吸氣狀態
    else if (isInhaling) {
        action = "attack"; frame = 0;
    }
    // 4. 地面蹲下
    else if (isDown && isOnGround) {
        action = "down"; frame = 0;
    }
    // 5. 地面衝刺 (檢查是否正在移動)
    else if (isDashing && vx != 0 && isOnGround) {
        action = "run";
        frameCounter++;
        frame = (frameCounter / 7) % 3 + 5; // 播放 5, 6, 7 幀
    }
    // 6. 地面走路
    else if (vx != 0 && isOnGround) {
        action = "run";
        frameCounter++;
        frame = (frameCounter / 10) % 3 + 1; // 播放 1, 2, 3 幀
    }
    // 7. 靜止
    else {
        action = "stop";
        frameCounter = 0;
        frame = 0;
    }

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
        setPixmap(pix); //把這個道具給演員本身
    }
}
