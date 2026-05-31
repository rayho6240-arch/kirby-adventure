# Qt Kirby Adventure Code Report

## Table of Contents

1. [Introduction](#1-introduction)
2. [Game Specification & Design](#2-game-specification--design)
3. [System Architecture](#3-system-architecture)
4. [Core Mechanism Implementation](#4-core-mechanism-implementation)
5. [Technical Highlights](#5-technical-highlights)
6. [Difficulties and Improvements](#6-difficulties-and-improvements)
7. [Conclusion](#7-conclusion)

---

## 1. Introduction

本專案是一款以 Qt Graphics View Framework 製作的 2D 橫向卷軸動作遊戲，主題參考《星之卡比》的核心玩法，包含角色移動、跳躍、飛行、吸入敵人、吐出星星、能力取得、Boss 戰、道具回復、場景切換與背景音樂等內容。

整體程式以 `QGraphicsScene` 作為遊戲世界，`QGraphicsView` 作為顯示與攝影機視角，並透過 `QTimer` 以固定時間間隔驅動主要遊戲迴圈。玩家、敵人、子彈、Boss、地形與道具皆以 `QGraphicsItem` 系列類別呈現，使顯示、碰撞與座標管理能整合在同一套 Qt 架構中。

[Insert Figure Here]

圖 1. 遊戲畫面總覽或開始畫面截圖

本報告著重於系統設計與核心機制，說明專案如何將角色控制、碰撞判定、斜坡地形、Boss 行為、音效播放、能力系統與場景切換整合為完整的遊戲流程。

---

## 2. Game Specification & Design

### 2.1 Game Overview

遊戲採用橫向卷軸關卡設計。玩家控制 Kirby 在場景中移動、跳躍、飛行與攻擊，避開或擊敗敵人，並在特定位置進入下一個場景。第二關加入斜坡、浮動平台、補血與加命道具，以及 Boss 戰，讓遊戲節奏從基本移動與敵人互動逐步進入較完整的戰鬥流程。

| 項目 | 設計內容 |
| --- | --- |
| 遊戲類型 | 2D 橫向卷軸動作遊戲 |
| 開發框架 | Qt / C++ |
| 核心視覺系統 | `QGraphicsScene`、`QGraphicsView`、`QGraphicsPixmapItem` |
| 更新方式 | `QTimer` 每 16 ms 觸發一次 game loop |
| 主要操作 | 左右移動、跳躍、飛行、蹲下、吸入、吐星、丟棄能力 |
| 關卡內容 | Start Menu、Stage 1、Stage 2、Game Over、Finish |
| 戰鬥要素 | 一般敵人、能力敵人、Boss、星星子彈、炸彈星 |

### 2.2 Player Controls

玩家輸入由 `MainWindow::keyPressEvent()` 與 `MainWindow::keyReleaseEvent()` 處理，再呼叫 `Kirby` 提供的公開介面改變角色狀態。這樣的設計讓輸入層與角色行為層分離，`MainWindow` 負責判斷按鍵，`Kirby` 負責真正的物理與狀態變化。

| 按鍵 | 功能 |
| --- | --- |
| Left / Right | 水平移動 |
| Left / Right 雙擊 | 衝刺 |
| Z | 跳躍 |
| Up | 一般狀態下跳躍或飛行操作 |
| Down | 蹲下；部分能力狀態下轉換型態 |
| X | 吸入、吐星或吐出炸彈星 |
| V | 丟棄能力；在浮動平台上可向下穿越 |
| Enter | 選單確認或重新開始 |

[Insert Figure Here]

圖 2. 操作流程或鍵盤控制示意圖

### 2.3 Level Design

遊戲目前以兩個主要關卡為核心。Stage 1 以基礎地面、牆面與敵人配置為主，讓玩家熟悉移動、吸入與吐星。Stage 2 擴充場景寬度，加入斜坡、浮動平台、道具與 Boss，使關卡結構更接近完整動作遊戲。

| 場景 | 主要內容 | 設計目的 |
| --- | --- | --- |
| Start Menu | 開始畫面與主題音樂 | 建立遊戲入口 |
| Stage 1 | 基礎地形、WaddleDee、WaddleDoo、Gordo、HotHead | 熟悉移動與基本戰鬥 |
| Stage 2 | 斜坡、浮動平台、Sparky、道具、Boss | 增加地形與戰鬥複雜度 |
| Game Over | Continue / Quit 選擇 | 處理失敗流程 |
| Finish | 結束動畫與音樂 | 完成遊戲回饋 |

---

## 3. System Architecture

### 3.1 Overall Architecture

專案採用以 `MainWindow` 為核心管理者的架構。`MainWindow` 建立場景、載入關卡、管理遊戲狀態、接收鍵盤輸入、執行 game loop，並維護玩家、敵人、子彈、炸彈、Boss 與 HUD 的容器。

[Insert Figure Here]

圖 3. 系統架構圖

| 模組 | 主要類別 | 責任 |
| --- | --- | --- |
| 遊戲主控 | `MainWindow` | 場景載入、game loop、輸入處理、狀態切換 |
| 玩家角色 | `Kirby` | 移動、跳躍、飛行、吸入、能力、受傷 |
| 地形 | `Block`、`Slope`、`FloatingPlatform` | 平台、牆壁、斜坡與特殊通過平台 |
| 敵人 | `Enemy`、`WaddleDee`、`WaddleDoo`、`HotHead`、`Sparky`、`Gordo` | 敵人 AI、移動、攻擊與可吸入行為 |
| Boss | `Boss`、`Bomb`、`BombStar` | Boss 狀態機、炸彈攻擊、Boss 戰傷害 |
| 投射物 | `StarBullet`、`BombStar`、`Fireball` | 玩家或敵人的遠距攻擊 |
| 道具 | `Item`、`MaximTomato`、`OneUp` | 補血、加命與消耗處理 |
| HUD | `HUD` | 顯示生命值、血量與遊戲狀態 |
| 音效 | `QMediaPlayer`、`QMediaPlaylist` | 背景音樂播放與場景音樂切換 |

### 3.2 Game Loop

主迴圈由 `QTimer` 每 16 ms 觸發，約等於 60 FPS。每次更新時會依序處理玩家、敵人、Boss、炸彈、子彈、碰撞、HUD 與攝影機位置。

主要流程如下：

1. 更新玩家狀態與吸入判定。
2. 更新所有敵人。
3. 更新 Boss，並接收 Boss 的炸彈生成請求。
4. 更新炸彈、炸彈星與一般星星子彈。
5. 判斷玩家與敵人、Boss、投射物之間的碰撞。
6. 更新 HUD 顯示。
7. 判斷生命值與 Game Over。
8. 讓攝影機跟隨玩家。

[Insert Figure Here]

圖 4. Game loop 更新流程圖

### 3.3 State Management

遊戲使用 `GameState` enum 管理目前場景，包括選單、關卡、Game Over 與結束狀態。當玩家在特定位置按下 Up 或 Enter 時，`MainWindow` 會呼叫對應的 `loadStage` 函式重建場景。

此設計的優點是流程清楚，每個場景的初始化集中在獨立函式中，例如 `loadStartMenu()`、`loadStage1()`、`loadStage2()`、`loadGameOver()` 與 `loadFinish()`。當切換場景時，程式會清除 scene、重置物件列表，再載入新的背景、地形、敵人、玩家與 HUD。

---

## 4. Core Mechanism Implementation

### 4.1 Character Movement

Kirby 的移動由速度、重力與狀態旗標共同控制。角色具有水平速度 `vx`、垂直速度 `vy`、重力 `gravity`，並透過 `isOnGround`、`isFlying`、`isDashing`、`isDown`、`isInhaling` 等狀態判斷可執行的動作。

| 機制 | 實作重點 |
| --- | --- |
| 水平移動 | 按左右鍵設定 `vx`，每幀更新 X 座標 |
| 衝刺 | 利用雙擊計時器判斷快速連按，將速度提高 |
| 跳躍 | 只有在地面且未吸入、未含物件時可設定向上速度 |
| 飛行 | 讓角色進入 `isFlying`，並套用較小重力與向上速度 |
| 蹲下 | 設定 `isDown`，停止衝刺，並可觸發部分能力變身 |
| 邊界限制 | 根據目前關卡寬度限制角色不能離開地圖 |

角色物理不是直接依賴 Qt 的內建物理引擎，而是在 `Kirby::update()` 中自行計算速度、位置與碰撞修正。這讓角色可以支援卡比遊戲常見的特殊行為，例如吸入時無法移動、飛行時重力變小、胖卡比狀態有較大的 hitbox。

[Insert Figure Here]

圖 5. Kirby 移動狀態與速度變化示意圖

### 4.2 Collision Detection

碰撞判定主要使用 Qt Graphics View 的 `collidingItems()` 與 `collidesWithItem()`。專案將碰撞分為 X 軸與 Y 軸處理，先更新水平位置並解決牆面碰撞，再更新垂直位置並處理落地、頂部碰撞、浮動平台與斜坡。

| 碰撞類型 | 判定方式 | 處理結果 |
| --- | --- | --- |
| 牆面碰撞 | X 軸移動後檢查 `Block` | 修正 X 座標並停止水平穿透 |
| 地面碰撞 | Y 軸下降時檢查 `Block` | 將角色放到地面上，`vy = 0` |
| 天花板碰撞 | Y 軸上升時檢查 `Block` | 停止向上速度並避免穿越 |
| 浮動平台 | 僅從上方落下時站立 | 可透過 Down + V 向下穿越 |
| 敵人碰撞 | 玩家與敵人 `collidesWithItem()` | 攻擊狀態擊殺敵人，否則受傷 |
| 子彈碰撞 | 子彈與敵人或 Boss 碰撞 | 子彈消失，目標受傷或死亡 |
| 道具碰撞 | bounding rect 交集 | 觸發補血或加命效果 |

玩家碰撞有一個值得注意的設計：顯示圖片大小與實際物理 hitbox 分開計算。程式會依照飛行、吐星、胖卡比等狀態調整物理寬高，再計算 offset，使視覺 sprite 與碰撞盒不必完全相同。這能讓動畫圖片較大時仍保持合理的操作手感。

[Insert Figure Here]

圖 6. Sprite 與實際 hitbox 關係示意圖

### 4.3 Slope System

斜坡系統由 `Slope` 類別實作，並繼承自 `Block`。不同於一般矩形地形，`Slope` 使用 `QPolygonF` 描述三角形或多邊形斜面。當 Kirby 與斜坡碰撞時，程式會根據角色腳底中心 X 座標計算斜坡表面的 Y 值，再把角色放置到該表面上。

斜坡高度計算的核心概念如下：

1. 將 Kirby 的 scene X 座標轉為斜坡 local X。
2. 遍歷 polygon 的每一條邊。
3. 找出 X 座標位於該邊範圍內的線段。
4. 使用線性插值計算該 X 對應的 Y。
5. 回傳 scene 座標中的表面高度。

| 設計項目 | 說明 |
| --- | --- |
| 地形表示 | 使用 `QPolygonF` 定義斜坡形狀 |
| 表面計算 | 以腳底中心 X 對斜坡邊線做線性插值 |
| 接觸判斷 | 角色下降且腳底穿越表面時視為落在斜坡 |
| 狀態紀錄 | 以 `MoveMode::SlopeMode` 標記正在斜坡上 |
| 容錯處理 | 使用 epsilon 與 fallback 避免邊界誤判 |

Stage 2 中大量使用斜坡組合地形，讓場景不只是平面平台，而能呈現更自然的上下坡路線。

[Insert Figure Here]

圖 7. 斜坡 polygon 與表面高度插值示意圖

### 4.4 Boss System

Boss 系統由 `Boss` 類別負責，採用有限狀態機設計。Boss 具有固定 arena 範圍、重力、速度、血量、動畫圖片與血條。其行為不是單一追蹤玩家，而是由數個狀態構成循環，使 Boss 戰有節奏感。

| Boss 狀態 | 行為 |
| --- | --- |
| `SmallHop` | 在 arena 內小跳移動 |
| `BigJumpToKirby` | 朝 Kirby 方向大跳 |
| `JumpBack` | 遠離 Kirby 後跳 |
| `VerticalHopPrepare` | 垂直跳起，準備投擲炸彈 |
| `DropBomb` | 完成炸彈投擲流程後回到小跳 |
| `Hurt` | 預留受傷狀態 |
| `Dead` | Boss 被擊敗並隱藏 |

Boss 投擲炸彈時不直接在自身類別中新增場景物件，而是設定 `bombSpawnRequested`，由 `MainWindow::gameLoop()` 呼叫 `consumeBombSpawnRequest()` 取得位置與速度，再建立 `Bomb` 物件。這種做法讓 Boss 專注於行為決策，而實際物件管理仍由主控場景負責。

玩家吸入 Boss 炸彈後，會進入 `hasBombInMouth` 狀態，再按攻擊鍵可吐出 `BombStar`。`BombStar` 撞到 Boss 時造成傷害並播放爆炸流程，形成 Boss 戰的主要攻擊循環。

[Insert Figure Here]

圖 8. Boss 狀態機與炸彈攻擊流程圖

### 4.5 Sound System

音效系統使用 `QMediaPlaylist` 與 `QMediaPlayer` 播放背景音樂。主選單、關卡與結束畫面分別加入 playlist 的不同 index，切換場景時透過 `setCurrentIndex()` 切換音樂，再呼叫 `play()` 播放。

| Playlist Index | 音樂用途 |
| --- | --- |
| 0 | Start Menu 主題音樂 |
| 1 | Stage 背景音樂 |
| 2 | Finish 結束音樂 |

程式也考慮了執行檔位置不同的問題，會嘗試兩種路徑尋找音樂檔案：一種是從 build 輸出資料夾往回找專案資料夾，另一種是直接從執行目錄中的 `bg_music` 資料夾讀取。這讓專案在 Qt Creator 或不同部署方式下較容易找到音樂資源。

[Insert Figure Here]

圖 9. 場景與背景音樂切換示意圖

### 4.6 Ability System

能力系統主要集中於 `Kirby`。角色透過吸入不同敵人改變 `currentForm` 與 `currentAbility`。一般敵人被吸入後可吐出星星；特定敵人則讓 Kirby 進入能力準備狀態，再透過蹲下轉換成正式能力型態。

| 敵人類型 | 吸入後型態 | 能力 |
| --- | --- | --- |
| 一般敵人 | Normal / mouthful | 可吐出 `StarBullet` |
| Sparky | `SparkyFat` -> `Sparky` | 電擊攻擊 |
| HotHead | `FireFat` -> `FireForm` | 火焰攻擊 |
| Boss 炸彈 | `hasBombInMouth` | 可吐出 `BombStar` 攻擊 Boss |

能力系統使用兩層狀態表示：

1. `CurrentAbility` 表示目前能力種類，例如 None、Spark、Fire、Beam。
2. `Form` 表示 Kirby 的實際外觀與動作型態，例如 Normal、SparkyFat、Sparky、FireFat、FireForm。

這樣能區分「已吸入但尚未正式變身」與「已取得能力並可攻擊」兩種狀態。玩家也能使用 V 丟棄能力，回到 Normal 狀態。

[Insert Figure Here]

圖 10. 能力取得與型態轉換流程圖

### 4.7 Scene Switching

場景切換由 `GameState` 與各個 `load...()` 函式控制。當玩家在 Stage 1 終點站在地面並按 Up，會切換到 Stage 2；當玩家在 Stage 2 終點且 Boss 已死亡，按 Up 會進入 Finish。

切換流程通常包含以下步驟：

1. 停止 timer，避免載入過程中持續更新。
2. 清除 `scene` 與物件列表。
3. 設定新的 `sceneRect`。
4. 加入背景圖片、地形、敵人、玩家、Boss、道具與 HUD。
5. 連接 Kirby 的 signal，例如 `starFired` 與 `bombStarFired`。
6. 設定音樂或保留目前音樂。
7. 重新啟動 timer。

Stage 1 切換到 Stage 2 時會保留 Kirby 的 HP 與 lives，讓玩家狀態能跨關卡延續。這使場景切換不是單純重開遊戲，而是具有連續性的關卡流程。

[Insert Figure Here]

圖 11. 場景切換流程圖

---

## 5. Technical Highlights

### 5.1 Separation Between Main Control and Entity Behavior

`MainWindow` 負責整體遊戲流程與物件生命週期，`Kirby`、`Enemy`、`Boss` 等類別則各自處理自身行為。這種分工讓 game loop 可以用統一方式呼叫各物件的 `update()`，同時保留不同角色的獨立邏輯。

### 5.2 Qt Signal/Slot for Projectile Management

Kirby 產生星星或炸彈星時，透過 `starFired` 與 `bombStarFired` signal 通知 `MainWindow` 將投射物加入管理列表。這避免 Kirby 直接操作主控列表，使角色類別不必知道太多場景管理細節。

### 5.3 Polygon-Based Slope Collision

斜坡不是用多個小矩形堆疊，而是以 polygon 表示，再用插值計算表面高度。這讓地形資料更接近實際形狀，也讓 Stage 2 的地形能自然上下坡。

### 5.4 Boss Finite State Machine

Boss 行為以明確狀態切換實作，比單純隨機移動更可控。每個狀態有自己的速度設定、落地後轉換規則與動畫更新方式，使 Boss 戰有可預期但仍具挑戰性的節奏。

### 5.5 Dynamic Hitbox Adjustment

Kirby 在不同狀態下會調整實際碰撞盒，例如飛行、吐星或胖卡比型態使用較大 hitbox，一般狀態使用較小 hitbox。這種做法讓角色圖片、動畫與操作手感可以分開調整。

### 5.6 Resource-Based Visual Assets

多數圖片透過 Qt resource path 載入，例如背景、Kirby sprite、敵人、Boss、道具與 HUD。這使專案資源能統一管理，也方便在不同執行環境中打包。

---

## 6. Difficulties and Improvements

### 6.1 Collision Accuracy and Sprite Size

本專案中角色圖片與碰撞尺寸不同，這是動作遊戲常見問題。若直接使用圖片 bounding rect，角色會因圖片透明區或動畫尺寸改變而產生卡牆、穿牆或落地不準。專案目前透過自訂 physical width / height 與 offset 修正此問題。

可改進方向：

| 問題 | 目前做法 | 改進建議 |
| --- | --- | --- |
| Sprite 尺寸與碰撞盒不同 | 手動設定物理寬高與 offset | 建立專用 hitbox component |
| 不同角色碰撞邏輯分散 | Kirby 與 Enemy 各自處理 | 抽出共用 collision helper |
| 特殊平台判斷較複雜 | 在 Kirby Y 軸碰撞中處理 | 將平台行為封裝到地形類別 |

### 6.2 Slope Edge Cases

斜坡系統需要處理角色剛好站在邊界、快速下落、離開斜坡等情況。專案使用 epsilon 與 `checkSlopeContact()` 補強判斷，減少角色在斜坡邊緣抖動或突然進入 NormalMode 的問題。

可改進方向是建立更完整的地形查詢系統，例如先用腳底探針取得下方地形，再根據地形類型決定落地高度，而不是完全依賴 `collidingItems()`。

### 6.3 Scene Loading and Object Lifetime

場景切換時會大量使用 `scene->clear()` 並清空多個 list。這種方式簡單有效，但隨著關卡變多，會讓初始化程式碼逐漸變長。

可改進方向：

1. 建立 `LevelData` 或 `StageBuilder`，把地形、敵人與道具配置資料化。
2. 將背景、地形、敵人生成拆成小函式。
3. 建立統一的 entity manager 處理新增、刪除與更新。

### 6.4 Audio Scalability

目前音樂系統能完成場景背景音樂切換，但主要集中在 BGM。若未來加入攻擊音效、受傷音效、道具音效與 Boss 音效，可以建立 SoundManager 統一管理短音效與背景音樂，避免各類別直接操作播放器。

### 6.5 Ability System Expansion

目前已支援 Spark、Fire 與預留 Beam enum。能力取得與型態切換已具備基礎架構，但不同能力的攻擊邏輯仍可能分散在 `Kirby` 或敵人類別中。

可改進方向：

| 改進目標 | 說明 |
| --- | --- |
| Ability class | 將不同能力封裝成獨立類別 |
| 統一攻擊介面 | 例如 `activate()`、`update()`、`cancel()` |
| 動畫資料化 | 將 sprite path 與 frame 規則從程式中抽出 |
| 冷卻時間 | 為能力加入更清楚的 cooldown 與 UI 顯示 |

### 6.6 Debug Output

專案中有許多 `qDebug()` 用於檢查 HP、座標、Boss 狀態與攻擊事件，開發時很有幫助。但若輸出過多，可能影響閱讀與效能。

建議後續加入 debug flag 或 log level，在正式展示時關閉高頻率輸出。

---

## 7. Conclusion

本專案完成了一個具備完整遊戲流程的 Qt 2D 動作遊戲原型。系統以 `MainWindow` 作為主控，透過 `QGraphicsScene` 管理場景物件，並以 `QTimer` 驅動穩定的 game loop。玩家角色 Kirby 擁有移動、跳躍、飛行、吸入、吐星、能力轉換與受傷復活等核心行為；敵人、Boss、道具、地形與 HUD 也各自形成清楚的模組。

從技術角度來看，本專案的重點不只是顯示角色與背景，而是實作了多個動作遊戲需要的核心系統：分軸碰撞判定、動態 hitbox、polygon 斜坡、Boss 有限狀態機、場景狀態切換、背景音樂播放，以及基於吸入行為的能力系統。這些設計讓遊戲已具備從開始、遊玩、戰鬥到結束的完整結構。

後續若要進一步提升，可優先將關卡資料、碰撞系統、音效系統與能力系統模組化，使新增關卡、敵人與能力時更容易維護。整體而言，本專案已展現 Qt/C++ 在 2D 遊戲開發上的可行性，也建立了可持續擴充的遊戲基礎。

[Insert Figure Here]

圖 12. 最終遊戲成果或系統整合展示圖
