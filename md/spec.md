# Kirby Adventure 專案規格書

## 1. 專案概述

本專案是一個以 C++ / Qt 製作的橫向卷軸動作遊戲，主題參考 Kirby's Adventure。玩家操作 Kirby 通過起始選單、Stage 1、Stage 2，最後進入 Finish 畫面；若生命歸零則進入 Game Over 畫面。

專案採用 Qt Widgets 與 Graphics View 架構，主要以 `QGraphicsScene` 管理場景物件，以 `QGraphicsView` 作為攝影機與顯示視窗，並透過 `QTimer` 以約 60 FPS 更新遊戲邏輯。

## 2. 開發與建置環境

- 語言：C++11
- 框架：Qt
- Qt 模組：
  - `core`
  - `gui`
  - `widgets`
  - `multimedia`
  - `multimediawidgets`
- 專案檔：`kirby-adventure.pro`
- 進入點：`main.cpp`
- UI 檔案：`mainwindow.ui`
- 資源檔：`res.qrc`

建置方式以 Qt Creator 開啟 `kirby-adventure.pro` 後編譯執行為主。

## 3. 專案檔案結構

主要原始碼檔案如下：

| 檔案 | 職責 |
| --- | --- |
| `main.cpp` | 建立 `QApplication` 與 `MainWindow`，啟動 Qt event loop |
| `mainwindow.h/.cpp` | 遊戲主控制器，負責場景切換、輸入、game loop、碰撞統整、BGM |
| `Kirby.h/.cpp` | 玩家角色，負責移動、跳躍、飛行、吸入、吐星、能力、HP / lives |
| `Enemy.h/.cpp` | 敵人抽象基底類別，提供共用物理與碰撞處理 |
| `WaddleDee.h/.cpp` | 一般巡邏敵人 |
| `Gordo.h/.cpp` | 固定位置障礙型敵人 |
| `HotHead.h/.cpp` | 火焰敵人，可提供 Fire 能力 |
| `Sparky.h/.cpp` | 電擊敵人，可提供 Spark 能力 |
| `StarBullet.h/.cpp` | Kirby 吐出的星星子彈 |
| `Fireball.h/.cpp` | HotHead 發射的火球 |
| `Block.h/.cpp` | 地形與碰撞方塊，使用 `QGraphicsPolygonItem` |
| `Slope.h/.cpp` | 斜坡地形 |
| `FloatingPlatform.h/.cpp` | 可站立、可下穿的平台 |
| `Item.h/.cpp` | 道具基底與 Maxim Tomato / 1UP |
| `Effect.h/.cpp` | 吸入、火焰、電擊等特效動畫 |
| `HUD.h/.cpp` | 顯示 HP 與 lives |
| `res.qrc` | Qt resource 清單，包含圖片素材 |
| `README.md` | 課程專案需求說明 |
| `DEVLOG.md` | 開發紀錄 |

素材主要位於 `Project2_Dataset/Image/`，BGM 與 finish animation 則由程式依執行檔路徑嘗試載入。

## 4. 系統架構

### 4.1 主架構

遊戲由 `MainWindow` 統一管理：

- 建立 `QGraphicsScene` 與 `QGraphicsView`
- 固定視窗尺寸為 `1620 x 1080`
- 使用 `QTimer` 每 16ms 呼叫 `gameLoop()`
- 管理遊戲狀態 `GameState`
- 管理玩家、敵人清單、星星子彈清單、HUD
- 處理鍵盤事件
- 切換 Start Menu、Stage 1、Stage 2、Game Over、Finish
- 播放 BGM

### 4.2 遊戲狀態

`GameState` 定義如下：

| 狀態 | 說明 |
| --- | --- |
| `STATE_MENU` | 起始選單 |
| `STATE_STAGE1` | 第一關 |
| `STATE_STAGE2` | 第二關 |
| `GAMEOVER` | 遊戲結束畫面 |
| `STATE_FINISH` | 通關結算 / 動畫畫面 |

基本流程：

```text
Start Menu -> Stage 1 -> Stage 2 -> Finish
                         |
                         v
                      Game Over
```

## 5. 遊戲畫面與攝影機

- 視窗固定大小：`1620 x 1080`
- Stage 1 場景寬度：`4860`
- Stage 2 場景寬度：`8100`
- 攝影機在 `gameLoop()` 中以 Kirby 的 X 座標為中心追蹤
- 攝影機 Y 中心固定為 `540`
- HUD 會依 Kirby 位置調整，保持在畫面上方附近

## 6. 輸入規格

| 按鍵 | 狀態 | 功能 |
| --- | --- | --- |
| Enter / Return | Start Menu | 進入 Stage 1 |
| Enter / Return | Game Over | 確認 Continue / Quit |
| Up | Game Over | 選擇 Continue |
| Down | Game Over | 選擇 Quit |
| Left / Right | Stage | 左右移動 |
| Left / Right 連按 | Stage | 衝刺 |
| Down | Stage | 蹲下；若 Kirby 為吸入能力預備狀態，轉換成能力型態 |
| Z | Stage | 跳躍 |
| Up | Stage | 飛行 / 上升；在指定傳送區域可進入下一狀態 |
| X | Stage | 吸入、吐星、能力攻擊 |
| V | Stage | 放棄能力；若蹲在浮動平台上則下穿平台 |

雙擊判定：

- `DOUBLE_TAP_WINDOW = 250ms`
- 左右鍵放開後在時間窗內再次按下同方向鍵，觸發 dash

## 7. 玩家 Kirby 規格

### 7.1 生命與血量

- 最大 HP：3
- 初始 HP：3
- 最大 lives：3
- 初始 lives：3
- 受傷時扣 1 HP
- HP 歸零時扣 1 life 並重設 HP
- HP 歸零且 lives 即將歸零時進入 Game Over
- 受傷後進入無敵狀態，計時約 120 frames
- 無敵狀態以透明度閃爍表示

### 7.2 移動與物理

- 水平速度由 `MainWindow` 按鍵事件設定
- 一般移動速度約為 `7`
- dash 速度約為 `12`
- 重力為 `0.8`
- 跳躍初速度約為 `-15`
- 飛行上升速度約為 `-8`
- 飛行時重力降低為一般重力的 `0.65`
- Stage 2 會呼叫 `changeWidth(8100)` 改變 Kirby 可移動地圖寬度

### 7.3 Kirby 型態

`Kirby::Form` 包含：

| 型態 | 說明 |
| --- | --- |
| `Normal` | 一般型態 |
| `SparkyFat` | 吸入 Sparky 後尚未吞下的胖型態 |
| `Sparky` | Spark 能力型態 |
| `FireFat` | 吸入 HotHead 後尚未吞下的胖型態 |
| `FireForm` | Fire 能力型態 |

`Kirby::CurrentAbility` 包含：

- `None`
- `Spark`
- `Fire`

### 7.4 吸入與吐星

- `X` 在普通狀態會呼叫 `startInhaling()`
- 吸入範圍約為寬 `300`、高 `100`
- 敵人進入吸入範圍時會被施加水平速度往 Kirby 方向移動
- 敵人靠近 Kirby 時會被視為吸入成功
- 吸入 Waddle Dee / Gordo：不取得能力
- 吸入 Sparky：進入 `SparkyFat`，再按 Down 轉為 `Sparky`
- 吸入 HotHead：進入 `FireFat`，再按 Down 轉為 `FireForm`
- 若嘴裡有物件，按 `X` 會產生 `StarBullet`

### 7.5 能力攻擊

Spark：

- `Sparky` 型態按 `X` 進入電擊攻擊
- 電擊範圍以 Kirby 周圍特效呈現
- 與敵人碰撞時使敵人死亡並隱藏

Fire：

- `FireForm` 型態按 `X` 進入噴火攻擊
- 火焰 hitbox 約為 `130 x 100`
- 火焰方向依 Kirby 面向決定
- 火焰命中敵人時使敵人死亡並隱藏

放棄能力：

- 按 `V` 呼叫 `discardAbility()`
- Fire 型態放棄能力時目前會額外產生一顆 `StarBullet`
- 狀態回到 `Normal`

## 8. 敵人規格

### 8.1 Enemy 基底類別

`Enemy` 繼承：

- `QObject`
- `QGraphicsPixmapItem`

提供：

- `virtual void update() = 0`
- `virtual void draw()`
- `virtual QString getEnemyType() const`
- `vx`, `vy`
- `isDead`
- `isInhaled`
- 共用 `handlePhysics(width, height)`

共用物理：

- 套用重力
- 與 `Block` 做 X / Y 碰撞
- 撞到牆會反向
- 落到地面會歸零 Y 速度

### 8.2 Waddle Dee

- 一般巡邏敵人
- 初始水平速度 `vx = -2.0`
- 使用 Waddle Dee 圖片序列更新動畫
- 掉落至過深位置時會重設到 `(800, 500)`
- 可被吸入

### 8.3 Gordo

- 固定障礙型敵人
- `vx = 0`
- 不主動移動
- 使用兩張圖片交替動畫
- `getEnemyType()` 回傳 `"Gordo"`

### 8.4 Sparky

- 可提供 Spark 能力
- 追蹤 Kirby 距離
- 狀態：
  - `IDLE`
  - `JUMPING`
  - `ATTACKING`
  - `BEING_INHALED`
- Kirby 距離小於等於約 `300` 時進入攻擊
- 否則週期性跳躍移動
- 可被吸入，吸入後 Kirby 可取得 Spark 能力

### 8.5 HotHead

- 可提供 Fire 能力
- 狀態：
  - `PATROL`
  - `CHASE`
  - `CHARGE`
  - `FIRE`
  - `COOLDOWN`
  - `BEING_INHALED`
- 參數：
  - 巡邏速度：`2.0`
  - 火焰近距離範圍：`120`
  - 偵測範圍：`500`
  - 追逐時間：約 `12 frames`
  - 蓄力時間：約 `30 frames`
  - 攻擊時間：約 `120 frames`
  - 冷卻時間：約 `30 frames`
- 近距離會使用火焰攻擊
- 中距離會發射 `Fireball`
- 可被吸入，吸入後 Kirby 可取得 Fire 能力

## 9. 子彈與攻擊物件

### 9.1 StarBullet

- 由 Kirby 吐星產生
- 依 Kirby 面向往左或往右移動
- 速度約為 `15`
- 超出 X 範圍 `0 ~ 4860` 後隱藏
- `MainWindow::gameLoop()` 負責檢查 StarBullet 與敵人碰撞
- 命中敵人後，敵人死亡並隱藏，星星隱藏

注意：目前 StarBullet 的邊界判定固定使用 `4860`，Stage 2 寬度為 `8100`，因此 Stage 2 遠端區域可能會提早清除星星。

### 9.2 Fireball

- 由 HotHead 發射
- 每 16ms 透過自身 `QTimer` 移動
- 速度約為 `14`
- 命中 Kirby 時造成 1 點傷害
- 命中 Block 或 Enemy 時消失
- 超出 X 範圍 `0 ~ 4860` 後消失

注意：Fireball 邊界同樣固定使用 `4860`，Stage 2 遠端區域可能不完整。

## 10. 地形與碰撞

### 10.1 Block

- 使用 `QGraphicsPolygonItem`
- 可建立矩形或自訂多邊形
- `type()` 回傳自訂 Type，供 `qgraphicsitem_cast<Block*>` 使用
- 主要作為地面、牆、天花板、階梯與隱形碰撞區

### 10.2 Slope

- 繼承 `Block`
- 支援以 polygon 建立斜坡
- `getSurfaceY(kirbyX)` 依 Kirby 的 X 座標計算斜坡表面 Y
- Kirby 在 Y 軸碰撞時會特別處理斜坡接觸

### 10.3 FloatingPlatform

- 繼承 `Block`
- 可作為單向平台
- Kirby 從上方落下可站上平台
- 蹲下且位於平台上時，按 `V` 可觸發下穿

## 11. 道具規格

`Item` 繼承：

- `QObject`
- `QGraphicsPixmapItem`

道具以 `Kirby::update()` 中掃描場景物件並檢查 `sceneBoundingRect()` 相交來觸發。

### 11.1 Maxim Tomato

- 圖片：`Project2_Dataset/Image/item/Maxim Tomato.png`
- 效果：呼叫 `kirby->restoreFullHP()`
- 使用後從 scene 移除並 `deleteLater()`

### 11.2 OneUp

- 圖片：`Project2_Dataset/Image/item/1UP.png`
- 效果：呼叫 `kirby->addLife(1)`
- lives 不超過 max lives
- 使用後從 scene 移除並 `deleteLater()`

## 12. HUD 規格

HUD 使用 `QGraphicsItemGroup` 與 `QGraphicsTextItem` 顯示：

```text
HP: current / max , Lives: current / max
```

目前 HUD 以文字顯示 HP 與 lives，並以紅色粗體字呈現。Game Over 時可顯示 `GAME OVER`，但實際 Game Over 畫面主要由背景圖控制。

## 13. 場景規格

### 13.1 Start Menu

- 場景大小：`1620 x 1080`
- 背景圖：`Project2_Dataset/Image/background/start.png`
- BGM：playlist index 0
- 按 Enter / Return 進入 Stage 1
- timer 停止

### 13.2 Stage 1

- 場景大小：`4860 x 1080`
- 背景：
  - `stage-1_1.png`
  - `stage-1-bg.jpg`
- 主要地形：
  - 地面、牆、天花板等隱形 `Block`
- 玩家初始位置：`(400, 100)`
- 敵人配置：
  - 3 個 Waddle Dee，位置約為 `(800, 500)`, `(1300, 500)`, `(1800, 500)`
  - 1 個 Gordo，位置約為 `(1200, 600)`
  - 1 個 HotHead，位置約為 `(2000, 500)`
- Stage 轉換：
  - Kirby 位於 X 約 `4600 ~ 4680` 且在地面，按 Up 進入 Stage 2
- BGM：playlist index 1

注意：依 README 原始需求 Stage 1 只應出現 Waddle Dee 與 Gordo，但目前程式實作 Stage 1 有加入 HotHead。

### 13.3 Stage 2

- 場景大小：`8100 x 1080`
- 背景：
  - `stage2_merged_nowater.jpg`
  - `stage-1-bg.jpg`
  - `stage2_augment.jpg`
- 地形：
  - 地面分段
  - 天花板
  - 多個階梯與牆
  - 11 個斜坡 polygon
  - 10 個浮動平台
- 玩家初始位置：`(400, 100)`
- 玩家 map width：`8100`
- 敵人配置：
  - 3 個 Sparky，位置約為 `(800, 500)`, `(1300, 500)`, `(1800, 500)`
- 道具配置：
  - Maxim Tomato：`(1600, 830)`
  - OneUp：`(1700, 830)`
- Stage 轉換：
  - Kirby 位於 X 約 `7800 ~ 7900` 且在地面，按 Up 進入 Finish
- BGM：playlist index 1

注意：目前 Stage 2 主要配置 Sparky，道具只會在 `maximTomatoSpawned` / `oneUpSpawned` 為 false 時生成。這兩個旗標在進入 Game Over 或回 Start Menu 時沒有看到重設邏輯，因此 Continue 後道具是否重生需再確認。

### 13.4 Game Over

- 場景大小：`1620 x 1080`
- 背景圖：
  - Continue：`game_over_continue.png`
  - Quit：`game_over_quit.png`
- Up 選 Continue
- Down 選 Quit
- Enter / Return 確認
- Continue：回 Start Menu
- Quit：呼叫 `qApp->quit()`
- timer 停止

### 13.5 Finish

- 場景大小：`1620 x 1080`
- 進入 Finish 時會依剩餘 HP / lives 計算 `remain_Hp`
- 依 `finish_animation/finish_%1/%2.png` 讀取逐幀圖片
- 使用 `finish_timer` 每 10ms 更新圖片
- 動畫播放完後回 Start Menu
- BGM：playlist index 2

## 14. 音效與音樂

使用：

- `QMediaPlaylist`
- `QMediaPlayer`

playlist index：

| Index | 用途 | 檔案 |
| --- | --- | --- |
| 0 | Start Menu | `bg_music/01_Main_Title.mp3` |
| 1 | Stage | `bg_music/91_Vegetable_Valley_(No Intro).mp3` |
| 2 | Finish | `bg_music/41_Goal.mp3` |

程式會嘗試從 build 目錄與專案目錄相對路徑尋找音樂檔。

## 15. 資源規格

圖片資源透過 `res.qrc` 編入 Qt resource system，使用 `:/Project2_Dataset/...` 路徑載入。

主要資源類型：

- Kirby normal 動畫
- Kirby fire 動畫
- Kirby spark 動畫
- Waddle Dee 動畫
- Gordo 動畫
- Hot Head 動畫
- Sparky 動畫
- 背景圖
- 道具圖
- 吸入特效
- Game Over 圖

部分外部資源未列入 `res.qrc`，由檔案系統載入：

- `bg_music/`
- `finish_animation/`

## 16. 目前實作注意事項

以下為掃描程式後觀察到的實作狀態，供後續維護參考：

- `kirby-adventure.pro` 的 `RESOURCES` 中重複加入了 `res.qrc`。
- `StarBlock.cpp` 為空檔案，`Starblock.h` 目前只宣告空類別或未完整實作。
- 多處註解出現亂碼，可能是原始檔案編碼與目前閱讀環境不一致。
- `Enemy::handlePhysics()` 與 `Fireball` / `StarBullet` 的 X 邊界仍固定為 `4860`，Stage 2 寬度 `8100` 時可能有行為不一致。
- Stage 1 目前實作包含 HotHead，與 README 描述的 Stage 1 敵人限制不完全一致。
- Stage 2 目前只生成 Sparky，未看到 Waddle Dee、Gordo、HotHead 的 Stage 2 配置。
- `loadStage2()` 會建立新的 Kirby 並繼承 Stage 1 的 HP / lives，但能力狀態不會繼承。
- `scene->clear()` 會清除場景物件，清單也會清空，但部分自行管理 timer 的物件需留意生命週期。
- `finish_timer` 每次進入 Finish 都 new 一個 timer，未看到明確刪除，因 parent 是 `this`，生命週期會跟隨 MainWindow，但多次進入 Finish 可能累積 timer 物件。
- `gameLoop()` 中有多個 `qDebug()` 每 frame 輸出，執行時可能造成 console 噪音或效能影響。

## 17. 延伸維護建議

- 將地圖寬度集中成場景或關卡設定，避免 `4860` 散落在 `Enemy`、`StarBullet`、`Fireball` 等類別。
- 將 Stage 1 / Stage 2 的敵人與地形配置資料化，減少硬編碼座標。
- 將 HP / lives HUD 改為圖片式顯示，與素材中的 `HP_0.png`、`HP_1.png`、`life.png` 對齊。
- 將 Game Over / Continue 流程重設道具生成旗標，避免重新開始後道具狀態不一致。
- 補上 Stage 2 多種類敵人配置，讓 Fire 與 Spark 能力來源更完整。
- 若要符合課程需求，需重新檢查 README 中每項 requirement 與目前實作是否一致。
