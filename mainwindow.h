#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// ==========================================
// 1. Qt 內建函式庫引入
// ==========================================
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QList>

#include <QMediaPlayer>
#include <QGraphicsVideoItem>

// ==========================================
// 2. 自定義遊戲類別引入
// ==========================================
#include "Kirby.h"
#include "WaddleDee.h"
#include "Gordo.h"
#include "HotHead.h"
#include "StarBullet.h"
#include "HUD.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; } //後面就不打 "mainwindow::"
QT_END_NAMESPACE

// [新增] 定義遊戲目前的狀態
enum GameState {
    STATE_MENU,
    STATE_STAGE1,
    STATE_STAGE2,
    GAMEOVER,
    STATE_FINISH
};

/**
 * @brief MainWindow 遊戲主視窗類別
 * @details 負責初始化遊戲場景、管理 Game Loop、調度所有遊戲實體（卡比、敵人、星星），
 * 並攔截玩家的鍵盤輸入事件。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //cotr,dtor
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // [註] Slots 讓這些函式可以被 connect() 連結，就像「收音機」監聽特定的「信號（Signal）」。

    /**
     * @brief 核心遊戲迴圈 (Game Loop)
     * @details 由主計時器 (timer) 驅動，每一幀都會呼叫此函式。
     * 負責更新卡比狀態、敵人移動、子彈飛行與碰撞偵測。
     */
    void gameLoop();

    /**
     * @brief 處理連按判定超時(衝刺有用到)
     * @details 當玩家按了一下方向鍵後，若在指定時間內沒有按第二下，此函式會重置判定狀態。
     */
    void onDoubleTapTimerTimeout();

private:
    // ==========================================
    // 1. UI 與場景管理 (Graphics & View)
    // ==========================================
    Ui::MainWindow *ui;
    QGraphicsScene *scene;  ///< 遊戲場景（所有物件都在這上面）
    QGraphicsView *view;    ///< 攝影機視角（玩家看到畫面的窗口）


    // ==========================================
    // 2. 遊戲介面 (Heads-Up Display / HUD)[新增]
    // ==========================================
    HUD *gameHUD; //創立物件




    // ==========================================
    // 3. 遊戲核心計時器 (Timers)~~
    // ==========================================
    QTimer *timer;          ///< 控制 Game Loop 的主計時器（例如設定每 16ms 觸發一次，達成 60FPS）

    // ==========================================
    // 4. 遊戲實體物件 (Game Entities)
    // ==========================================
    Kirby *player;                  ///< 玩家操作的卡比
    

    /**
     * @brief 敵軍陣列
     * @note 這裡使用多型 (Polymorphism)，以 Enemy* 基底指標儲存，
     * 這樣陣列裡就可以同時存放 WaddleDee 或未來新增的各種敵人。
     */
    QList<Enemy*> enemyList;

    QList<StarBullet*> bulletList;  ///< 管理畫面上所有正在飛行的星星子彈


    // ==========================================
    // 5. 遊戲狀態 (Game States)(卡比有沒有死)[新增]
    // ==========================================
    bool isGameOver=false;  ///< 記錄是否已經死亡或遊戲結束，避免在 GameLoop 中繼續執行邏輯
    
    bool cont = true;


    // ==========================================
    // 6. 玩家輸入與狀態判定 (Input & States)
    // ==========================================
    // [註] 鍵盤雙擊判定保留在此，因為這是玩家「硬體輸入層級」的邏輯，判定完再將結果傳給 Kirby。
    int lastReleasedKey = -1;          ///< 記錄上一個放開的方向鍵鍵值（用來比對是否連按同一個鍵）
    QTimer *doubleTapTimer;            ///< 雙擊判定專用的計時器
    const int DOUBLE_TAP_WINDOW = 250; ///< 雙擊判定的有效時間差（250毫秒內連按兩下才算衝刺）


    // =========================================================
    // [新增] 場景設定
    // =========================================================
    GameState currentState; // 記錄現在是在選單還是關卡中
    void loadStartMenu();
    void loadStage1();
    void loadStage2();
    void loadGameOver();
    void loadFinish();

    // [待修改] 結算畫面的影片播放參數，但目前無法播放
    QMediaPlayer* m_finishPlayer;
    QGraphicsVideoItem* m_videoItem;

    QGraphicsPixmapItem* gameover;

    // 將血量寫在header file避免重複寫
    int c_Hp = 3;
    int c_lives = 3;

protected:
    // ==========================================
    // 系統事件覆寫 (Event Overrides)
    // ==========================================

    /**
     * @brief 攔截鍵盤「按下」事件
     * @param event 包含玩家按下了哪顆按鍵的資訊
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 攔截鍵盤「放開」事件
     * @param event 包含玩家放開了哪顆按鍵的資訊
     */
    void keyReleaseEvent(QKeyEvent *event) override;

};

#endif // MAINWINDOW_H
