# Bug Fix / Debug 開發日記


---

## 1. StarBullet 撞牆後不再穿牆

- 日期：2026-06-02
- commit hash：`7bd31b32baeec8fe579240cff58b63c0c1f54aa2`
- 作者：`rayho6240@gmail.com`
- commit message：`fix: "修復星星撞牆問題"`
- 分類：collision bug / projectile

### 問題現象

Kirby 吐出的 `StarBullet` 在碰到 `Block` 或牆壁時可能穿過地形。直接套用 Fireball 的碰撞方式時，又會因為星星圖片和碰撞框太大，導致星星剛生成就被地板或附近 Block 消掉。

### 原因推測

從 diff 可以確認，原本 `StarBullet` 的視覺圖片範圍與碰撞範圍沒有分離。星星圖片很大，如果直接使用整張 pixmap 的 shape 做碰撞，會把視覺透明區或外圍也算進碰撞，造成誤判。

### 修復方式

在 `StarBullet` 新增 `shape()` override，使用 `boundingRect().adjusted(...)` 建立較小的實際 hitbox。`update()` 中改用 `scene()->collidingItems(this)` 檢查碰撞，並在碰到 `Block` 時將星星隱藏，交由原本 `bulletList` 清理流程移除。

### 穩定性影響

這個修復讓 projectile 與地形互動更一致，避免星星穿牆，也避免星星出生點太靠近 Kirby 或地面時被錯誤刪除。對戰鬥與關卡碰撞穩定性影響高。


---

## 2. 變身 Kirby 長按 Up 無法自動拍翅

- 日期：2026-06-02
- commit hash：`000be08287c481b452e90cff0cfa301bf6ffebcf`
- 作者：`rayho6240@gmail.com`
- commit message：`fix: "修復變身卡比長按 Up 無法自動拍翅"`
- 分類：UI input conflict / ability state

### 問題現象

Normal Kirby 可以長按 Up 自動拍翅，但 Sparky、FireForm、BeamForm 等變身狀態無法使用同樣的長按飛行手感。

### 原因推測

從 diff 可確認，原本 auto-flap 判斷寫死 `currentForm == Form::Normal`。`MainWindow::keyPressEvent()` 中 Up 鍵流程也只對 Normal 設定 `setUpPressed(true)`，因此其他型態即使具備飛行動作，也不會進入持續拍翅流程。

### 修復方式

新增 `Kirby::canAutoFlap()`，集中判斷 Normal、Sparky、FireForm、BeamForm 是否可自動拍翅。`Kirby::update()` 的 auto-flap 條件改用 `canAutoFlap()`，`MainWindow::keyPressEvent()` 的 Up 鍵處理也改用同一個介面。

### 穩定性影響

這讓不同能力型態的輸入行為統一，減少每個型態各自寫判斷造成的遺漏。對操作一致性與能力狀態維護有正面影響。


---

## 3. Enemy / Boss / Item 不可以站在斜坡上

- 日期：2026-06-01
- commit hash：`49ded095c284ae1c87a8d2e41f4c49d1f758950f`
- 作者：`rayho6240@gmail.com`
- commit message：`把beam 所有圖摳好+敵人與boss與item 可站在斜坡上`
- 分類：collision bug

### 問題現象

Kirby 已經可以使用 `Slope::getSurfaceY()` 站在斜坡上，但 Enemy、Boss、Item 仍可能把斜坡當牆，或只支援平面 `Block` 落地，導致它們無法自然貼著斜坡站立。

### 原因推測

從 diff 可確認，`Enemy::handlePhysics()` 原本對 X 軸碰撞直接把所有 `Block` 類物件當牆處理，而 `Slope` 繼承自 `Block`。因此敵人接觸斜坡時，可能先被 X 軸碰撞邏輯反彈。Y 軸落地也沒有使用斜坡表面高度。Item 原本沒有獨立更新地形物理；Boss 的 `standingY()` 則只依固定 `groundY`。

### 修復方式

`Enemy.cpp` 在 X 軸碰撞時跳過 `Slope`，Y 軸碰撞時使用 `slope->getSurfaceY(footCenterX)` 對齊腳底。`Item` 新增簡單重力與地形落地邏輯，並在 `MainWindow::gameLoop()` 更新場景中的 Item。`Boss::standingY()` 改為若腳下有斜坡，優先使用斜坡表面高度。

### 穩定性影響

這讓不同遊戲物件共用相同的斜坡概念，避免 Kirby 能站、其他物件卻穿透或彈開的地形不一致。對 Stage 2 這類大量斜坡地圖的穩定性很重要。


---

## 4. Bomb 吸入流程避免碰到 Kirby 時爆炸

- 日期：2026-05-31
- commit hash：`98b45f8d89a1ff3065b998df9b0007ecc589f24b`
- 作者：`rayho6240@gmail.com`
- commit message：`實作bomb 可以被順利吸入+吐出bombStar打到boss 會切換成爆炸照片`
- 分類：collision bug / object lifetime / boss behavior

### 問題現象

Boss 丟出的 Bomb 在 Kirby 吸入時，可能尚未完成吸入就因為碰到 Kirby 本體而觸發爆炸。BombStar 打到 Boss 後也需要顯示爆炸圖，而不是立刻從畫面消失。

### 原因推測

從 diff 可確認，Bomb 原本只有一般物理、爆炸與 dead 狀態。沒有「正在被吸入」的中間狀態，因此 Bomb 接近 Kirby 時仍會進入一般 `collidesWithItem(player)` 爆炸分支。

### 修復方式

在 `Bomb` 中新增 `beingInhaled`、`startInhale()`、`isBeingInhaled()`、`moveToward()`。Bomb 進入吸入狀態後停止原本物理與碰撞爆炸，由 `MainWindow::gameLoop()` 將它拉向 Kirby 嘴巴，距離足夠近才完成吸入並移除。`BombStar` 也新增 `exploding` 與 `startExplosion()`，命中 Boss 後先顯示爆炸圖，再由原本清理流程移除。

### 穩定性影響

修復了吸入與傷害判定的優先順序問題，避免玩家明明在吸 Bomb 卻被炸到。也讓 BombStar 命中 Boss 的視覺回饋更清楚。


---

## 5. Normal Kirby 飛行時穿過天花板

- 日期：2026-05-28
- commit hash：`8f2ecba265b4f6a8e56b08efa07efd4d26312431`
- 作者：`rayho6240@gmail.com`
- commit message：`fix: 修復 Normal Kirby 飛行穿過天花板問題`
- 分類：collision bug / UI input conflict

### 問題現象

Normal Kirby 長按 Up 飛行時，撞到天花板後仍可能繼續往上移動，甚至出現 `y = -220` 等異常座標，看起來像被傳送回出生點。

### 原因推測

diff 和 commit message 都指出，Y 軸碰撞雖然會把 `vy` 設成 0，但 auto-flap 在同一 frame 後段仍可能因為 Up 持續按住而再次呼叫 `fly()`，把 `vy` 改回負值。當 Kirby 已經跑到天花板上方後，`collidingItems()` 可能無法再偵測原本的 ceiling block。

### 修復方式

在 `Kirby::update()` 新增 `hitCeilingThisFrame`。撞到天花板時把 Kirby 推回天花板下方、停止向上速度、設定 `autoFlapCooldown`，並 `break` 離開碰撞處理。另加 top-boundary guard，若 `sceneBoundingRect().top() < 0` 就推回畫面內。auto-flap 條件也加入 `!hitCeilingThisFrame`。

### 穩定性影響

這個修復避免玩家逃逸出場景邊界，也讓長按 Up 的輸入與碰撞處理不再互相打架。對角色物理穩定性影響高。


---

## 6. 吐星後仍可觸發變身

- 日期：2026-05-28
- commit hash：`3170a00976d196cdb52bf7fa342802effe2d7809`
- 作者：`rayho6240@gmail.com`
- commit message：`fix: 修復吐星後仍可變身的狀態殘留問題`
- 分類：ability / skill menu state

### 問題現象

Kirby 吐出 StarBullet 後，按 Down 仍可能觸發變身，像是嘴裡的能力狀態沒有被完全清掉。

### 原因推測

從 diff 可確認，原本 `Kirby::spit()` 只清除 `hasObjectInMouth` 和 full 狀態，但沒有把 `currentForm` 與 `currentAbility` 回復為 Normal / None。若吐星前處於 `SparkyFat` 或 `FireFat` 這類 pending ability 狀態，後續 Down 鍵仍可能被判定為可變身。

### 修復方式

在 `Kirby::spit()` 的恢復正常狀態流程中加入：

```cpp
currentForm = Form::Normal;
currentAbility = CurrentAbility::None;
```

### 穩定性影響

這是小 patch，但對能力狀態機很關鍵。它避免吐星與變身兩條流程互相污染，讓 Down 鍵重新回到蹲下語意。



---

## 7. 深淵掉落後扣血與重生狀態清理

- 日期：2026-05-28
- commit hash：`bb9d941c4b25a23f15e4410be3a59dd1fb8b07cf`
- 作者：`rayho6240@gmail.com`
- commit message：`fix: 新增深淵掉落扣血與重生機制`
- 分類：scene switching bug / object lifetime / cleanup

### 問題現象

Kirby 掉出場景後，若只重設位置，可能保留掉落速度或暫時狀態，導致重生後又繼續下墜、狀態異常或受傷流程不一致。

### 原因推測

從 diff 可確認，修復前 HP 歸零後的流程直接使用 `setPos(400,100)`。這只改位置，不會清除 `vx`、`vy`、飛行、蹲下、衝刺、吸入、平台下穿等暫時狀態。

### 修復方式

新增 `Kirby::respawnAt(qreal x, qreal y)`，統一重生時要做的清理：重設位置、速度、蹲下 / 飛行 / 衝刺 / 吸入 / spitting / platform flags，並給短暫無敵。`MainWindow::gameLoop()` 新增 `player->y() > 1400` 的深淵判定，掉落時呼叫既有 `takeDamage(1)`，若尚未 Game Over 則 `respawnAt(400,100)`。

### 穩定性影響

重生流程變得集中且可預期，避免 scene 中位置重設與角色狀態清理分散在不同地方。對 Game Over、生命、重生體驗都有幫助。


---

## 8. Normal Kirby 蹲下抖動

- 日期：2026-05-27
- commit hash：`ee3bfff9a6a2c9f66b980b48d866fa73020ac4e8`
- 作者：`rayho6240@gmail.com`
- commit message：`fix: 修復 Normal Kirby 蹲下抖動問題`
- 分類：collision bug / UI input conflict

### 問題現象

Normal Kirby 按住 Down 時，角色會在站立與蹲下 sprite 之間快速切換，看起來像抖動。Fire / Spark 蹲下沒有同樣問題。

### 原因推測

從 diff 和 commit message 可確認，Normal crouch sprite 更換 pixmap / alpha mask 後，`QGraphicsPixmapItem` 的碰撞形狀跟著改變。物理判定依賴 `collidingItems()` 與 `isOnGround`，蹲下瞬間可能短暫失去地板接觸，下一 frame 又切回站立，於是反覆抖動。

### 修復方式

在 `Kirby::updateSprite()` 中加入 Normal crouch 穩定化處理。Normal 且按住 Down 時改用 `BoundingRectShape`；實際選到 `down` action 時記錄舊的腳底 bottom Y，換圖與 offset 後再修正 `y()`，讓腳底位置保持固定。

### 穩定性影響

修復後，角色圖片切換不再破壞地板接觸判定。這讓動畫、碰撞與輸入狀態的關係更穩定，也降低玩家操作時的視覺抖動。


---

