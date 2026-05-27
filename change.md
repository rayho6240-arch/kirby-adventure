# 變更紀錄

## Normal 蹲下抖動修復

### 修改檔案

- `Kirby.cpp`

### 修改函式

- `Kirby::updateSprite()`

### 問題原因

Normal Kirby 切換蹲下圖片時，`QGraphicsPixmapItem` 會跟著 pixmap / alpha mask 改變碰撞形狀。因為目前物理判定依賴 `collidingItems()`，而蹲下動作只有在 `isDown && isOnGround` 時才會被選中，所以 Normal 蹲下圖片可能短暫失去地板接觸。

下一幀失去 `isOnGround` 後，角色又會切回站立或非蹲下圖片；站立圖片恢復地板碰撞後，又再次進入蹲下。這就造成「站立 / 蹲下」快速來回切換。

### 最小修補

- Normal 且按住 Down 時，改用 `QGraphicsPixmapItem::BoundingRectShape` 作為碰撞形狀。
- Normal 實際選到 `down` 動作時，換圖後修正 `y()`，讓 Kirby 腳底保持固定。
- 加入一次性的 `qDebug()`：`Normal crouch stabilizer active`。

## 深淵掉落扣血與重生

### 修改檔案

- `Kirby.h`
- `Kirby.cpp`
- `mainwindow.cpp`

### 修改函式

- `Kirby::respawnAt()`
- `MainWindow::gameLoop()`

### 最小修補

- 在 `MainWindow::gameLoop()` 檢查 Kirby 是否 `y() > 1400`。
- 掉入深淵時輸出 `qDebug() << "Kirby fell into abyss";`。
- 掉入深淵時呼叫現有 `player->takeDamage(1)` 扣 1 HP，沿用既有無敵與 HP / lives 邏輯。
- 若不是最後一命 Game Over，將 Kirby 送回目前關卡出生點 `(400, 100)`。
- 新增 `Kirby::respawnAt()`，重設位置、`vx`、`vy`、蹲下 / 飛行 / 衝刺 / 吸入等暫時狀態，並給予短暫無敵。
- HP 歸零後的既有重生流程也改用 `respawnAt(400, 100)`，避免重生後保留掉落速度。

### 影響範圍

- 沒有重寫 physics。
- 沒有新增大型 respawn system。
- 沒有改 state machine。
- 沒有改 scene 架構。
