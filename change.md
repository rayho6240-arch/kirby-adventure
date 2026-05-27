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

