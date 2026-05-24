#ifndef KIRBY_H
#define KIRBY_H

// ==========================================
// 1. Qt 內建函式庫引入
// ==========================================
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QString>
#include <QPixmap>
#include <QImage>
#include <QRectF>
#include <QObject>

// ==========================================
// 2. 自定義遊戲類別引入
// ==========================================
#include "Enemy.h"
#include "StarBullet.h"
#include "Effect.h"

/**
 * @brief Kirby 玩家主角類別
 * @details 負責處理卡比的狀態機（走、跑、跳、飛）、物理運動、動畫切換以及專屬能力（吸入/吐出）。
 * @note 為了使用 Signal/Slot 機制，必須同時繼承 QObject 與 QGraphicsPixmapItem，
 * 並且 QObject 必須放在第一個繼承順位。
 */
class Kirby : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT // <--- 必須加在類別定義的第一行，這樣 Qt 的 MOC 編譯器才會處理它的 Signal/Slot！

signals:
    // ==========================================
    // 訊號發送區 (Signals)
    // ==========================================
    /**
     * @brief 卡比吐出星星時發送的訊號
     * @param star 卡比動態產生的星星子彈物件指標
     * @note 這是 Qt 最優雅的解法，讓 Kirby 負責產生物件，然後「喊一聲」，
     * 讓 MainWindow 透過 Slot 把它撿起來放進更新與渲染清單，達成解耦。
     */
    void starFired(StarBullet* star);

public:


    enum class CurrentAbility {
        None,
        Spark,
        Fire
    };

    enum class Form {
        Normal,     // 普通狀態
        SparkyFat,  // 吃掉 Sparky 變胖了 (準備變身)
        Sparky,     // 已經獲得 Spark 技能 (變身完成)
        FireFat,    // 吃掉 HotHead 變胖了 (準備變身)
        FireForm    // 已經獲得 Fire 技能 (變身完成)
    };

    CurrentAbility currentAbility = CurrentAbility::None;





    // ==========================================
    // 生命週期與核心迴圈 (Lifecycle & Core)
    // ==========================================
    Kirby(); //ctor

    /**
     * @brief 卡比的核心物理與動畫更新邏輯
     * @details 由 MainWindow 的 Game Loop 每一幀呼叫。負責套用重力、更新座標、切換 Sprite。
     */
    void update();

    // ==========================================
    // 玩家動作指令區 (Movement Commands)
    // 供 MainWindow 攔截鍵盤輸入後呼叫
    // ==========================================
    void jump();                            ///< 執行跳躍 (賦予一個向上的初速度)
    void fly();                             ///< 執行飛行 (拍打翅膀，賦予向上的升力)
    void setHorizontalVelocity(qreal v);    ///< 設定水平移動速度 (左右移動)
    void setDashing(bool dashing);          ///< 切換是否處於衝刺狀態
    void setDown(bool down);                ///< 切換是否處於蹲下狀態
    qreal getVx() const { return vx; }      ///< 取得卡比目前的 X 軸速度，供外部判斷面向方向用

    // ==========================================
    // 戰鬥與互動邏輯 (Combat & Interaction)
    // ==========================================
    /**
     * @brief 處理按下攻擊鍵(X)的總體邏輯
     * @details 卡比會自行根據當前狀態判斷：如果嘴裡沒東西就開始吸，有東西就吐出來。
     */
    void handleAttack();

    void startInhaling();                   ///< 開始吸氣動畫與狀態
    void stopInhaling();                    ///< 停止吸氣動畫與狀態
    void spit();                            ///< 吐出星星攻擊

    /**
     * @brief 處理吸入敵人的範圍判定
     * @param enemies 傳入畫面上所有敵人的清單 (傳參考)
     * @details 會在卡比面前畫一個「看不見的長方形Hitbox」，只要敵人在裡面，就會受到吸引力並被吃掉。
     */
    void processInhale(QList<Enemy*> &enemies);
    void discardAbility();                  ///< 棄置目前能力，恢復 Normal 形態

    //[新增]生命機制
    int getCurrentHp() const { return currentHp; }
    int getMaxHp() const { return maxHp; }

    //[新增]lives機制
    int getCurrentlives() const { return currentlives; }
    int getMaxlives() const { return maxlives; }


    //控制血量與生命
    void minusCurrentlives(int live = 1) { currentlives -= live; }
    void setCurrentHp(int hp = 3) { currentHp = hp; }
    void setCurrentlives(int live) { currentlives = live; }

    void restoreFullHP();             ///< 恢復當前 HP 到最大值
    void addLife(int amount = 1);      ///< 增加剩餘生命

    void takeDamage(int damage); // 承受傷害的函式
    bool getInhaling();          //讓外面得到資訊(傷害判定)
    bool getSpitting();
    bool getHasObjectInMouth() const { return hasObjectInMouth; } // 讓外部知道卡比嘴裡有沒有東西
    bool isSparkyElectricAttack() const { return currentForm == Form::Sparky && isInhaling; }
    
    // [新增] 得到是否在地上的資訊
    bool getOnGround();
    bool getDown() const { return isDown; }
    bool isOnFloatingPlatform() const { return onFloatingPlatform; }
    void setPassThroughPlatform(bool pass) { passThroughPlatform = pass; }
    // [新增] 改變卡比邊界檢查的範圍
    void changeWidth(int width);


private:

    Form currentForm = Form::Normal; // 預設是普通型態




    // ==========================================
    // 內部狀態變更與渲染 (Private Helpers)
    // ==========================================
    /**
     * @brief 處理卡比「滿嘴/空嘴」的狀態切換
     * @param full true表示吞入敵人肚子變大，false表示吐出恢復原狀
     */
    void setFullStatus(bool full);

    /**
     * @brief 更新卡比的圖片 (Sprite)
     * @details 根據當前的各項 boolean 狀態，決定要播放哪一張動畫幀。
     */
    void updateSprite();
    void applyFlameDamage();
    bool isFireBreathAttack() const { return currentForm == Form::FireForm && isInhaling; }

    // ==========================================
    // 物理屬性參數 (Physics Parameters)
    // ==========================================
    qreal vx = 0;                           ///< 當前 X 軸速度 (像素/幀)
    qreal vy = 0;                           ///< 當前 Y 軸速度 (像素/幀)
    const qreal gravity = 0.8;              ///< 重力加速度
    const qreal DASH_SPEED = 12;            ///< 衝刺時的固定速度

    // ==========================================
    // 狀態機變數 (State Flags)
    // ==========================================
    // 動作狀態
    bool isFacingRight = true;              ///< 是否面向右邊
    bool isDown = false;                    ///< 是否正在蹲下
    bool isFlying = false;                  ///< 是否處於飛行/膨脹狀態
    bool isDashing = false;                 ///< 是否正在衝刺
    bool isOnGround = false;                ///< 是否確實踩在實體地板上

    // 戰鬥狀態
    bool isInhaling = false;                ///< 是否正在吸氣
    bool hasObjectInMouth = false;          ///< 肚子裡/嘴裡是否有東西 (滿腹狀態)
    bool isSpitting;  // 記錄是否正在吐星
    int spitTimer;    // 吐星狀態的持續時間計時器

    //生命狀態
    int maxHp;              // 最大血量
    int currentHp;          // 當前血量
    
    int maxlives;
    int currentlives;

    bool isInvincible;      // 是否處於無敵狀態
    int invincibleTimer;    // 無敵時間倒數計時器

    // ==========================================
    // 動畫計時器 (Animation Counters)
    // ==========================================
    int frameCounter = 0;                   ///< 記錄一般動畫經過的幀數
    int flapCounter = 0;                    ///< 記錄飛行拍動翅膀的幀數
    const qreal flameHitboxWidth = 130.0;   ///< 火焰判定區寬度
    const qreal flameHitboxHeight = 100.0;  ///< 火焰判定區高度
    
    
    // [新增] 控制地圖寬度的變數，到了stage2可用changeWidth(int width)改變
    int mapwidth = 4860;

    // [新增] 特效物件
    Effect *fireEffect = nullptr;
    Effect *sparkEffect = nullptr;
    Effect *inhaleEffect = nullptr; ///< 動態產生的吸氣特效（parent 設為 Kirby）

    // [新增] 單向平台狀態與穿過旗標
    bool onFloatingPlatform = false;
    bool passThroughPlatform = false;

};

#endif // KIRBY_H
