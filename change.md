# Change Log

## Normal crouch jitter fix

### Files changed

- `Kirby.cpp`

### Functions changed

- `Kirby::updateSprite()`

### Root cause

Normal Kirby's crouch sprite changes the pixmap / alpha mask used by `QGraphicsPixmapItem` collision. Because the physics step depends on `collidingItems()` and the crouch action is selected only when `isDown && isOnGround`, the Normal crouch image could briefly lose ground contact. The next frame then selected the standing / non-crouch sprite, restoring contact, which caused rapid stand-crouch flicker.

Fire / Spark crouch did not show the same issue because their crouch sprites have different dimensions / masks and stayed more stable against the floor collision.

### Minimal patch

- When Kirby is in `Form::Normal` and Down is held, `updateSprite()` now uses `QGraphicsPixmapItem::BoundingRectShape` for the collision shape.
- When the selected Normal action is actually `down`, `updateSprite()` records Kirby's previous bottom Y and adjusts `y()` after `setPixmap()` / `setOffset()` so the feet stay anchored.
- Added a small one-time `qDebug()` message: `Normal crouch stabilizer active`.

### Scope

- No physics rewrite.
- No state machine rewrite.
- No changes to `MainWindow::keyPressEvent()` / `keyReleaseEvent()`.
- Fire / Spark crouch behavior is preserved outside the Normal crouch stabilizer path.
