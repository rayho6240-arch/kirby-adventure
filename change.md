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

## Fix Normal Kirby ceiling collision and auto-flap escape

### Changed file

- `Kirby.cpp`

### Problem

Normal Kirby could fly into the ceiling while holding Up. After touching the ceiling, Kirby could continue moving above the visible screen, with debug output showing positions such as `y = -220`. In some cases this later looked like a teleport back to the spawn point, but the root issue was that Kirby was escaping above the ceiling instead of being stopped by collision.

### Cause

- Normal auto-flap runs after the Y-axis collision handling in `Kirby::update()`.
- Ceiling collision set `vy = 0`, but holding Up could immediately trigger `fly()` again and set `vy = -8`.
- The ceiling collision branch did not stop further Y-collision processing.
- If Kirby had already moved above the ceiling, `collidingItems()` could stop detecting the ceiling block, so the normal ceiling collision branch no longer pulled Kirby back down.

### Fix

- Added a per-frame `hitCeilingThisFrame` flag in `Kirby::update()`.
- When Kirby hits the ceiling:
  - place Kirby back below the ceiling using `sceneBoundingRect()` / `boundingRect().top()`;
  - set `vy = 0`;
  - set `hitCeilingThisFrame = true`;
  - set a short `autoFlapCooldown`;
  - `break` out of Y-collision handling.
- Added a small top-boundary guard: if Kirby's `sceneBoundingRect().top()` becomes negative, push him back below the top edge and stop upward velocity.
- Normal auto-flap now checks `!hitCeilingThisFrame` before calling `fly()`.

### Notes

- Did not modify `respawnAt()`.
- Did not modify abyss death handling.
- Did not change Fire / Spark flight behavior.
- Did not rewrite the physics or collision system.
