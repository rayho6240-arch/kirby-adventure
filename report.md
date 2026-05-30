# 遊戲規格與設計說明

## 專案架構摘要

本專案是一個使用 Qt Widgets / QGraphicsScene 製作的 2D 橫向卷軸遊戲。專案使用 `kirby-adventure.pro` 管理 qmake 建置，主要程式碼集中在根目錄的 `.cpp`、`.h` 檔案，圖片資源由 `res.qrc` 匯入，實際素材放在 `Project2_Dataset/Image`。

主要檔案與職責如下：

- `main.cpp`：建立 `QApplication` 與 `MainWindow`。
- `mainwindow.cpp/.h`：管理 `QGraphicsScene`、`QGraphicsView`、game loop、鍵盤輸入、關卡載入、Game Over、Finish 畫面與背景音樂。
- `Kirby.cpp/.h`：玩家角色，包含移動、跳躍、飛行、蹲下、吸入、吐星、受傷、生命值、能力狀態與動畫切換。
- `Enemy.cpp/.h`：敵人基底類別，提供簡單重力與碰撞處理。
- `WaddleDee`、`WaddleDoo`、`Sparky`、`HotHead`、`Gordo`：不同敵人行為。
- `Block`、`Slope`、`FloatingPlatform`：地形與碰撞物件。
- `Item`：道具系統，目前有 `MaximTomato` 與 `OneUp`。
- `HUD`：顯示血量與生命數圖片。
- `Effect`：吸入、火焰、電擊等動畫效果。
- `StarBullet`、`Fireball`：玩家吐出的星星與 HotHead 發射的火球。

遊戲主流程是從開始畫面按 Enter 進入 Stage 1，玩家移動到 Stage 1 右側門口並按 Up 切換到 Stage 2。Stage 2 到達終點門口後按 Up 進入結算動畫，動畫播放完回到開始畫面。若 HP 與 lives 用盡，會進入 Game Over 畫面，可以選擇繼續回開始畫面或離開遊戲。

## 遊戲目標

玩家操作 Kirby 通過兩個橫向卷軸關卡，避開或擊敗敵人，抵達關卡出口。Stage 1 的目標是走到右側門口進入 Stage 2；Stage 2 的目標是到達終點門並進入 Finish 結算動畫。

## 操作方式

- 方向鍵 Left / Right：左右移動。
- 連續按同方向鍵兩次：進入衝刺，速度比一般移動快。
- Z：跳躍。
- Up：Normal 狀態下可跳躍或在空中持續飛行；在能力狀態下呼叫 `fly()`；在門口時切換場景。
- Down：蹲下；若 Kirby 嘴裡含有 Sparky 或 HotHead，蹲下會轉換成對應能力。
- X：攻擊鍵。嘴裡沒有東西時吸入，嘴裡有東西時吐出星星。
- 放開 X：停止吸入。
- V：若蹲在浮動平台上，會往下穿過平台；否則丟棄目前能力。
- Game Over 畫面：Up / Down 選擇繼續或離開，Enter 確認。

## 角色、敵人、障礙物與關卡設定

玩家角色是 `Kirby`。Kirby 有 HP 與 lives，初始最大 HP 為 3，最大 lives 為 3。角色能左右移動、跳躍、飛行、蹲下、吸入敵人、吐出星星，並可藉由吞下特定敵人取得能力。

敵人包含：

- `WaddleDee`：基本移動敵人，繼承 `Enemy` 的物理處理，會左右行走並播放走路動畫。
- `WaddleDoo`：靠近玩家時進入攻擊狀態，使用自訂 `shape()` 加入光束碰撞範圍。
- `Sparky`：有 IDLE、JUMPING、ATTACKING、BEING_INHALED 狀態。靠近玩家時會攻擊，平常會跳躍移動。Kirby 吸入後可取得 Spark 能力。
- `HotHead`：有 PATROL、CHASE、CHARGE、FIRE、COOLDOWN、BEING_INHALED 狀態。會偵測玩家距離，近距離噴火，較遠距離發射 `Fireball`。Kirby 吸入後可取得 Fire 能力。
- `Gordo`：固定不移動，只播放兩張圖片的動畫。

障礙物與地形包含：

- `Block`：矩形或多邊形碰撞地形。
- `Slope`：斜坡，透過 `getSurfaceY()` 依照 Kirby 腳下 x 座標計算斜面高度。
- `FloatingPlatform`：繼承自 `Block` 的單向平台，Kirby 可以站在上面，也可以蹲下按 V 往下穿過。

道具包含：

- `MaximTomato`：碰到後呼叫 `Kirby::restoreFullHP()` 回復滿 HP。
- `OneUp`：碰到後呼叫 `Kirby::addLife(1)` 增加生命數，但生命數不超過最大值。

## 勝利與失敗條件

勝利條件是在 Stage 2 終點門附近，玩家站在地上且 y 座標符合條件時按 Up，進入 `STATE_FINISH`。程式會依照剩餘 HP 與 lives 計算 `remain_Hp`，選擇不同的 finish animation 資料夾播放。

失敗條件是 Kirby 的 HP 歸零且 lives 也不足以繼續。此時 `MainWindow::gameLoop()` 會停止遊戲並呼叫 `loadGameOver()`。如果 HP 歸零但還有 lives，會扣一條命並重生到指定座標。

另外，Stage 1 或 Stage 2 中如果 Kirby 掉到 y > 1400，程式會把 HP 設為 0，視剩餘 lives 決定重生或 Game Over。

## 遊戲流程

1. 程式啟動後建立 `MainWindow`，初始化 `QGraphicsScene`、`QGraphicsView`、timer、playlist 與 BGM player。
2. 呼叫 `loadStartMenu()` 顯示開始畫面並播放主選單音樂。
3. 在開始畫面按 Enter 後進入 `loadStage1()`。
4. Stage 1 建立背景、地形、玩家、HUD 與敵人，並啟動 16 ms 的 `QTimer` 作為 game loop。
5. 玩家到 Stage 1 右側門口並按 Up 後進入 `loadStage2()`。
6. Stage 2 建立較長地圖、斜坡、浮動平台、道具與 Sparky 敵人。
7. 玩家到 Stage 2 終點門並按 Up 後進入 `loadFinish()`，播放結算動畫。
8. Finish 動畫結束後回到開始畫面。
9. 若 HP 與 lives 用盡，進入 `loadGameOver()`，可選擇繼續或離開。

# 核心機制實作說明

## 角色移動

鍵盤輸入由 `MainWindow::keyPressEvent()` 與 `keyReleaseEvent()` 處理。左右鍵會呼叫 `Kirby::setHorizontalVelocity()` 設定水平速度，一般速度為 7，雙擊方向鍵時速度為 12 並設定 `isDashing`。放開方向鍵時速度歸零，並啟動 `doubleTapTimer` 記錄雙擊時間窗。

Kirby 的實際位移在 `Kirby::update()` 中處理。程式先根據 `isDown`、`isInhaling` 等狀態決定本幀可用的水平速度，再做 X 軸移動與碰撞修正。

## 跳躍與重力

`Kirby::jump()` 在角色站地、沒有吸入、嘴裡沒有東西時，把 `vy` 設成 -15。`Kirby::fly()` 則將 `isFlying` 設為 true，並把 `vy` 設成 -8。

在 `Kirby::update()` 中，每幀都會套用重力。飛行狀態下重力為 `gravity * 0.65`，一般狀態下是 `gravity`。當玩家按住 Up，且 Normal 狀態、在空中、沒有碰到天花板、沒有吸入或含東西時，程式會用 `autoFlapCooldown` 週期性呼叫 `fly()`，做出連續拍翅上升的效果。

敵人的重力與基本碰撞主要在 `Enemy::handlePhysics()` 中處理。

## 碰撞判定

專案主要使用 Qt Graphics View 的碰撞系統，例如 `scene()->collidingItems(this, Qt::IntersectsItemShape)`、`collidesWithItem()` 與 `sceneBoundingRect()`。

Kirby 的碰撞處理分成 X 軸與 Y 軸：

- X 軸碰撞會檢查 `Block`，遇到牆時依照 Kirby 中心與 block 中心修正位置，避免穿牆。
- Y 軸碰撞會判斷落地、撞天花板、斜坡、浮動平台。
- `Slope` 會用多邊形邊線計算腳底所在位置的 surface y，讓 Kirby 可以貼著斜面站立。
- `FloatingPlatform` 只在 Kirby 從上方落下時承接，若 `passThroughPlatform` 為 true，則會讓 Kirby 往下掉。

道具碰撞使用 `sceneBoundingRect().intersects(item->sceneBoundingRect())`，碰到後呼叫各道具的 `onConsumed()`。

敵人與玩家的碰撞在 `MainWindow::gameLoop()` 中處理。若 Kirby 沒有吸入、沒有吐星、也不是 Spark 電擊攻擊，碰到敵人會呼叫 `takeDamage(1)`。Spark 電擊狀態下碰到敵人會直接讓敵人死亡並隱藏。

## 動畫切換

Kirby 的動畫集中在 `Kirby::updateSprite()`。它根據多個狀態決定 action 與 frame，例如 jump、fly、inhale、attack、down、run、stop。不同型態使用不同資料夾：

- Normal：`Project2_Dataset/Image/Kirby_normal`
- Spark：`Project2_Dataset/Image/Kirby_spark`
- Fire：`Project2_Dataset/Image/Kirby_fire`

Kirby 會依狀態設定圖片高度，Normal 大多縮放到 100，高階能力縮放到 140。程式也有針對 Normal 蹲下抖動做修正：蹲下時改用 `BoundingRectShape`，並在切換蹲下圖片後鎖定底部位置，避免 pixmap / alpha mask 改變造成短暫失去地面碰撞。

敵人各自管理動畫：

- `WaddleDee` 依照 `vx` 選擇左右走路 frame。
- `Gordo` 每隔固定 frame 在兩張圖間切換。
- `Sparky` 依狀態切換走路或攻擊圖片。
- `HotHead` 依 PATROL、CHASE、CHARGE、FIRE 等狀態切換圖片與火焰效果。
- `WaddleDoo` 使用 walk frame 與 attack frame，攻擊時還會改變顯示比例與碰撞 shape。

## 敵人或 Boss 行為

目前程式沒有實作 Boss 類別。敵人行為主要由多個小型 AI 組成：

- `WaddleDee` 是基本巡邏敵人。
- `WaddleDoo` 靠近玩家 280 距離內進入攻擊，攻擊 60 frame 後冷卻 60 frame。
- `Sparky` 每 60 frame 判斷玩家距離，距離 300 內攻擊，否則跳躍移動。
- `HotHead` 偵測 500 距離內玩家，靠近時先追蹤、蓄力，再進入 FIRE。距離 120 內使用火焰，120 到 500 之間發射 `Fireball`。
- `Gordo` 是固定障礙型敵人，只播放動畫。

## 場景切換

場景切換由 `MainWindow` 的 `GameState` 與各 `load...()` 函式管理：

- `loadStartMenu()`
- `loadStage1()`
- `loadStage2()`
- `loadGameOver()`
- `loadFinish()`
- `loadStage3()`
- `loadStage4()` 宣告存在，但目前只看到 `loadStage3()` 有空的雛形，`loadStage4()` 沒有實作內容。

每次切換關卡時，程式會停止 timer、`scene->clear()`、清空 `bulletList` 與 `enemyList`，再重新建立背景、地形、玩家、敵人與 HUD。

## 音效或資源管理

圖片資源由 `res.qrc` 管理，包含 Kirby 多種型態、敵人、背景、道具、HUD、Game Over 畫面等素材。

音樂使用 `QMediaPlaylist` 與 `QMediaPlayer`。程式會嘗試從執行檔目錄或專案相對路徑讀取：

- `bg_music/01_Main_Title.mp3`
- `bg_music/91_Vegetable_Valley_(No Intro).mp3`
- `bg_music/41_Goal.mp3`

不過目前掃描到的專案目錄只有 `.vs` 與 `Project2_Dataset`，沒有看到 `bg_music` 資料夾。因此音樂播放邏輯存在，但目前資源似乎沒有一起放在專案中。

Finish 動畫同樣使用外部資料夾 `finish_animation/finish_1` 到 `finish_7`，程式會逐張讀取 png frame。掃描到的專案檔案清單沒有看到 `finish_animation` 資料夾，因此這部分也需要確認部署時是否有額外提供。

## Game Loop / Update 機制

主要 game loop 是 `MainWindow::gameLoop()`，由 `QTimer` 每 16 ms 呼叫一次，約等於 60 FPS。

每次更新流程包含：

1. 更新玩家 `player->update()`。
2. 處理 Kirby 吸入敵人 `player->processInhale(enemyList)`。
3. 更新所有敵人 `e->update()`。
4. 檢查玩家與敵人碰撞。
5. 更新星星子彈，移除超出範圍或不可見的子彈。
6. 檢查星星子彈與敵人碰撞。
7. 更新 HUD。
8. 檢查死亡、扣命、Game Over。
9. 讓 camera 跟隨玩家，使用 `view->centerOn()`。

# 困難、限制與改進分析

## 可能遇到的困難

這個專案的困難主要在於 Qt 的 `QGraphicsPixmapItem` 會受 pixmap 與 shape mode 影響碰撞形狀。程式中已經可以看到 Normal 蹲下抖動修正：蹲下時將 shape mode 改成 `BoundingRectShape`，並鎖住底部位置。這表示開發過程中很可能遇到「換圖後碰撞範圍變動，導致站地狀態不穩」的問題。

另一個困難是角色狀態很多：移動、跳躍、飛行、蹲下、吸入、含敵人、吐星、能力型態、受傷無敵、平台穿越、斜坡模式都集中在 `Kirby::update()` 與 `updateSprite()`。這些狀態彼此會互相影響，容易產生判斷順序問題。

## 目前架構限制

- `Kirby::update()` 功能過多，同時處理物理、碰撞、道具、受傷無敵、飛行、特效與動畫觸發。
- `Kirby::updateSprite()` 使用大量 if/else 判斷狀態，後續若再增加能力或動作，維護成本會變高。
- `MainWindow::gameLoop()` 也負責太多工作，包含玩家、敵人、子彈、HUD、死亡流程與 camera。
- 關卡是直接在 `loadStage1()`、`loadStage2()` 中用程式碼手動建立，地形座標與敵人位置都寫死，之後新增或調整關卡會比較辛苦。
- `Enemy::handlePhysics()` 的地圖寬度寫死為 4860，Stage 2 寬度是 8100，敵人在 Stage 2 若需要完整地圖邊界行為，可能會不一致。
- `StarBlock.h/.cpp` 目前沒有實作內容，屬於未完成或保留功能。
- `STATE_STAGE3`、`STATE_STAGE4` 有列在 enum 中，但實際流程沒有完成。
- 音樂與 finish animation 是用外部路徑讀取，目前沒有在掃描到的專案資料夾中看到對應資料夾，部署時可能會缺檔。
- `kirby-adventure.pro` 的 `RESOURCES` 重複加入了兩次 `res.qrc`。
- 程式中有大量 `qDebug()`，對開發除錯有幫助，但正式報告或展示時可能會讓輸出太雜。

## 未來可改進方向

最適合改進的是狀態管理。Kirby 目前用許多 bool 與 enum 混合管理狀態，例如 `isFlying`、`isDown`、`isInhaling`、`hasObjectInMouth`、`isSpitting`、`currentForm`。未來可以改成 FSM，把「地面移動、跳躍、飛行、吸入、含敵人、能力攻擊、受傷」分成明確狀態，讓每個狀態只處理自己的輸入、物理與動畫。

關卡資料也可以抽出成資料檔，例如用 JSON 或簡單文字格式存地形、敵人、道具座標。這樣 `loadStage1()`、`loadStage2()` 就不用寫大量 new block 的程式碼。

碰撞部分可以進一步區分「視覺圖片」與「物理 hitbox」。現在 Kirby 還是依賴 `QGraphicsPixmapItem` 的碰撞，再用 offset 與 shape mode 修正。若未來做一個獨立的物理矩形，圖片只負責顯示，會比較穩定。

敵人也可以統一管理生命週期。目前敵人死亡多半是 `setVisible(false)` 與 `setIsDead(true)`，但不一定從 scene 或 list 移除。未來可以設計 entity manager，統一處理新增、更新、刪除。

# 技術亮點

## 1. 使用 Qt Graphics View 製作橫向卷軸場景

實作方式：專案使用 `QGraphicsScene` 放置背景、地形、玩家、敵人、子彈與 HUD，並用 `QGraphicsView::centerOn()` 讓鏡頭跟隨玩家。Stage 1 場景寬 4860，Stage 2 場景寬 8100。

困難點：`QGraphicsView` 的座標、scene rect、物件 z value 與 camera 位置要互相配合，尤其 HUD 需要用 `mapToScene()` 放在畫面固定區域。

學到什麼：學到 Qt 不只可以做表單 UI，也能用 scene / item 的方式做 2D 遊戲原型，並理解畫面座標與世界座標的差異。

## 2. Kirby 多狀態動作與能力系統

實作方式：`Kirby` 使用 `Form` enum 區分 Normal、SparkyFat、Sparky、FireFat、FireForm，並用 `currentAbility` 記錄能力。吸入 Sparky 或 HotHead 後先變成 fat 狀態，按 Down 後轉換成 Spark 或 Fire 能力。

困難點：移動、吸入、含敵人、能力攻擊、飛行、蹲下會互相影響，判斷順序需要很小心。例如吸入時不能移動，嘴裡有東西時不能跳或飛。

學到什麼：角色能力系統不能只看單一按鍵，需要把角色目前狀態一起考慮，否則很容易出現互相衝突的操作。

## 3. 吸入與吐星攻擊

實作方式：Kirby 按 X 時呼叫 `handleAttack()`，沒有東西時進入吸入，嘴裡有東西時建立 `StarBullet`。吸入範圍用 `QRectF inhaleRect` 判斷，把敵人的 `vx` 往 Kirby 方向拉，接近後隱藏敵人並改變 Kirby 型態。

困難點：吸入不是單純碰撞，而是需要方向、距離、敵人被吸入速度與吞下距離一起配合。左右方向的範圍也不同，需要額外調整。

學到什麼：遊戲互動常常需要自己設計 hitbox，不能完全依賴圖片碰撞。吸入這種技能需要把「判定範圍」和「視覺效果」分開思考。

## 4. 斜坡與單向平台

實作方式：`Slope` 繼承 `Block`，用多邊形表示斜面，再用 `getSurfaceY()` 根據 Kirby 腳底 x 座標計算應該站立的 y。`FloatingPlatform` 也繼承 `Block`，Kirby 從上方落下時才能站上去，蹲下按 V 可以穿越。

困難點：斜坡不能只靠矩形碰撞，否則角色會卡在斜面裡或浮在空中。單向平台則要判斷上一幀位置、落下方向與是否正在 pass through。

學到什麼：平台遊戲的地形判定比想像中複雜，尤其是斜坡和單向平台，都需要額外的物理邏輯。

## 5. WaddleDoo 自訂攻擊碰撞形狀

實作方式：`WaddleDoo` override `shape()`、`paint()` 與 `boundingRect()`。平常用橢圓作為本體碰撞，攻擊時用 `addBeamShape()` 根據攻擊 frame 加入光束矩形。

困難點：顯示圖片縮放、左右翻轉、碰撞 shape 與物理檢查會互相影響。程式中使用 `inPhysicsCheck` 讓物理碰撞時維持 100x100，避免攻擊光束影響行走碰撞。

學到什麼：Qt 的 item shape 可以做出比矩形更細的攻擊判定，但也要小心它同時會影響一般碰撞。

## 6. Finish 動畫依剩餘生命選擇

實作方式：到達 Stage 2 終點後，程式用剩餘 HP 與 lives 算出 `remain_Hp`，再轉成 `finish_1` 到 `finish_7` 的動畫資料夾，使用 `QTimer` 每 10 ms 更新圖片 frame。

困難點：這類逐張圖片播放需要處理路徑、frame 編號、總 frame 數與讀檔失敗。程式有用 `QFile::exists()` 在執行檔路徑與專案相對路徑之間選擇。

學到什麼：動畫不一定只能放在 qrc，也可以用外部資料夾逐張讀取，但部署時要確保資料夾有一起提供。

# 心得與反思

這次專案最大的收穫，是把平常課堂上學到的 C++ class、繼承、多型、事件處理，真的放進一個會動的遊戲裡。像 `Enemy` 作為基底類別，讓不同敵人都可以放進 `QList<Enemy*>` 裡更新，這是以前只看範例時比較沒有感覺的地方。

開發過程中也發現，遊戲程式很多問題不是語法錯，而是狀態之間互相影響。例如 Kirby 蹲下時圖片換了，碰撞形狀也跟著變，結果角色會短暫判斷成沒有站在地上。這種 bug 很難只靠看程式想出來，通常要一邊玩、一邊印 debug 訊息、一邊調整。

目前專案已經有基本的關卡、敵人、道具、能力與場景切換，但架構還有很多可以整理的地方。尤其 `Kirby::update()` 和 `MainWindow::gameLoop()` 都變得很大，很多判斷集中在一起。之後如果繼續做，會希望把角色狀態改成更明確的 FSM，也把關卡資料從程式碼中抽出來，這樣新增關卡或敵人會比較輕鬆。

整體來說，這個 project 讓我比較清楚理解遊戲不是只有畫面和按鍵，還包含資源管理、碰撞、動畫、狀態切換、生命週期管理等很多小系統。雖然目前還不是很完整，但已經把一個 2D 平台遊戲的主要骨架做出來，也讓我知道後續要怎麼把程式整理得更穩。
