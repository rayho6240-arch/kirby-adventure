//結構清晰：卡比怎麼跳、怎麼變胖都在 Kirby.cpp 裡。
//容易擴展：之後你要做敵人（Waddle Dee），只要再創一個 Enemy 類別，
//              並在 gameLoop 裡加上 enemy->update() 就好了。
//

//========================================================
#include "Kirby.h"         // 引入卡比自己的標頭檔 (必須放在最前面)
#include "Block.h"         // [關鍵新增] 因為我們要用 qgraphicsitem_cast 辨識 Block

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
    if (y() >= 800 && !isInhaling) {
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
//==============================================================
void Kirby::update() {
    // 1. 記錄移動前的狀態
    qreal oldY = y();

    // 2. 物理運算：計算這一幀應該要有的垂直速度 (vy)
    if (isFlying) vy += (gravity * 0.65);
    else vy += gravity;

    // 蹲下或吸氣時不能左右移動
    qreal currentVx = (isDown || isInhaling) ? 0 : vx;

    // 3. 計算預計位置 (計算完先不 setPos)
    qreal nextX = x() + currentVx;
    qreal nextY = y() + vy;

    // 水平邊界檢查 (維持 4860)
    nextX = qBound(0.0, nextX, 4860.0 - boundingRect().width());

    // 4. 正式套用位置 (只呼叫這一次 setPos)
    setPos(nextX, nextY);

    // 5. 碰撞偵測與修正
    const QList<QGraphicsItem *> collidingItems = scene()->collidingItems(this);
    isOnGround = false;

    // 定義一個固定的邏輯高度，避免因為換圖片(PNG透明邊緣不同)導致抖動
    const qreal KIRBY_PHYSICAL_HEIGHT = 80;

    for (QGraphicsItem *item : collidingItems) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block) {
            // 使用固定的 KIRBY_PHYSICAL_HEIGHT 代替 boundingRect().height()
            if (vy >= 0 && (oldY + KIRBY_PHYSICAL_HEIGHT <= block->y() + 20)) {
                setY(block->y() - KIRBY_PHYSICAL_HEIGHT);
                vy = 0;
                isOnGround = true;
                break;
            }
        }
    }

    // 6. 狀態補強
    if (isOnGround) {
        isFlying = false;
    }




    if (currentVx > 0) {
        // 速度大於 0，代表向右移动，面向右邊
        isFacingRight = true;
    } else if (currentVx < 0) {
        // 速度小於 0，代表向左移动，面向左邊
        isFacingRight = false;
    }

    // 7. 最後才更新動畫
    updateSprite();
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
