//
//
//
//

//========================================================


#include "Kirby.h"

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
    // [修改2] 這邊的舞台寬度在 stage 1 為 4860 pixels，stage 2 為 8100 pixels
    qreal nextX = qBound(0.0, x() + currentVx, 4860.0 - boundingRect().width());
    qreal nextY = y() + vy;

    // [修改] 下面的 if 攔截可以註解掉了，因為 qBound 已經處理好
    /*
    // 左邊界檢查 (0)
    if (nextX < 0) { ... }
    */

    // 套用位置
    setPos(nextX, nextY);

    // 2. 落地判定 (或將 800 改為變數以便維護)
    if (y() >= 800) {
        setY(800);
        vy = 0;
        isFlying = false; // [關鍵新增] 一旦碰到地面，強制解除變胖/飛行狀態
    }

    // 3. 更新朝向
    // 就算蹲下不能走，原地按左右鍵還是可以轉頭（因為 vx 還是有值）
    if (vx > 0) isFacingRight = true;
    else if (vx < 0) isFacingRight = false;

    // 4. 更新動畫
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
    QString dir = isFacingRight ? "R" : "L"; //宣告+定義變數 dir(direction)
    QString action;
    int frame = 0;

    // [修改] 這裡的 if-else 順序非常重要！決定了動畫的「優先權」後續可能用 RTOS架構之類的
    if (isDown && y() >= 800) {
        // [新增] 地面蹲下：蹲下圖 (優先級最高，蹲下就不能播跑步)
        action = "down"; frame = 0;
    }
    else if (isInhaling) {
        // [新增] 吸氣動畫優先級也很高！
        action = "attack"; frame = 0;
    }
    else if (isFlying) {
        // [修改] 飛行時的動畫判斷
        action = "fly";
        if (flapCounter > 0) {
            flapCounter--; // 計數器遞減
            frame = 2; // 顯示翅膀拍下的狀態 (第 2 張)
        } else {
            frame = 1; // 沒按按鍵時，維持滑翔/展翅狀態 (第 1 張)
        }
    }
    else if (y() < 800) {
        // [修改] 區分空中上升與下落
        action = "jump";
        if (vy > 0) frame = 3; // 下落中 (vy > 0)
        else frame = 1;        // 上升中 (vy <= 0)
    }
    else if (isDashing && vx != 0) {
        // [衝刺] 地面衝刺：我們讓標準跑圖循環快兩倍
        // 基於你給的圖kirby_run_4，我們恆定顯示衝刺圖的單幀手感通常更好：
        action = "run";
        frameCounter++;
        frame = (frameCounter / 7) % 3 + 5;
    }
    else if (vx != 0) {
        // 地面移動：跑圖
        action = "run";
        frameCounter++;
        frame = (frameCounter / 10) % 3 + 1;
    }
    else {
        // 地面靜止：站立圖
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
