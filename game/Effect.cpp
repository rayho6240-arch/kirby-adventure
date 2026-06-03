#include "Effect.h"
#include <QImage>
#include <QPainter> // 👈 引入 QPainter，解決 incomplete type 編譯錯誤

Effect::Effect(EffectType type, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent),
      currentFrame(0),
      frameDelay(0),
      counter(0),
      isLoop(false),
      isMirrored(false),
      finished(false),
      currentType(type) {
    
    if (type == EffectType::Inhale) {
        setEffect(":/Project2_Dataset/Image/inhalingBubble/inhale_smoke(%1).png", 5, 8, 160);
        setLoop(true);
    } else if (type == EffectType::Fire) {
        setEffect(":/Project2_Dataset/Image/Kirby_fire/kirbyfire_fire(%1)_R.png", 3, 4, 200);

        setLoop(true);
    } else if (type == EffectType::Spark) {
        setEffect(":/Project2_Dataset/Image/Kirby_spark/Kirby_spark_attack(%1).png", 3, 4, 250);
        setLoop(true);
    } else if( type == EffectType::Beam) {
        buildBeamEffect();
    }
}

Effect::Effect(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent),
      currentFrame(0),
      frameDelay(0),
      counter(0),
      isLoop(false),
      isMirrored(false),
      finished(false),
      currentType(EffectType::Inhale){
}

void Effect::setEffect(QString pathPattern, int frameCount, int delay, int targetHeight) {
    frames.clear();
    mirroredFrames.clear();
    currentFrame = 0;
    counter = 0;
    frameDelay = delay;
    finished = false;

    // 自動偵測圖片索引是從 0 開始還是從 1 開始
    int startIdx = 1;
    QPixmap firstCheck(pathPattern.arg(0));
    if (!firstCheck.isNull()) {
        startIdx = 0;
    }

    for (int i = 0; i < frameCount; ++i) {
        int idx = startIdx + i;
        QString path = pathPattern.arg(idx);
        QPixmap pix(path);
        if (!pix.isNull()) {
            if (targetHeight > 0) {
                pix = pix.scaledToHeight(targetHeight, Qt::SmoothTransformation);
            }
            frames.append(pix);
            
            // 預先生成鏡像圖片以提升效能
            QImage img = pix.toImage();
            QImage mirroredImg = img.mirrored(true, false);
            mirroredFrames.append(QPixmap::fromImage(mirroredImg));
        }
    }

    // 更新為初始影格
    updatePixmap();
}

void Effect::updateAnimation() {
    if (frames.isEmpty()) return;

    counter++;
    if (counter >= frameDelay) {
        counter = 0;
        currentFrame++;
        if (currentFrame >= frames.size()) {
            if (isLoop) {
                currentFrame = 0;
            } else {
                currentFrame = frames.size() - 1;
                finished = true;
                setVisible(false); // 播放完畢自動隱藏
                return;
            }
        }
        updatePixmap();
    }
}

void Effect::setMirror(bool mirrored) {
    if (isMirrored != mirrored) {
        isMirrored = mirrored;
        updatePixmap();
    }
}

void Effect::setLoop(bool loop) {
    isLoop = loop;
}

bool Effect::getLoop() const {
    return isLoop;
}

bool Effect::isFinished() const {
    return finished;
}

void Effect::updatePixmap() {
    if (frames.isEmpty()) return;

    // 防禦性邊界檢查
    if (currentFrame < 0 || currentFrame >= frames.size()) {
        currentFrame = 0;
    }

    if (isMirrored) {
        if (currentFrame < mirroredFrames.size()) {
            setPixmap(mirroredFrames[currentFrame]);
        }
    } else {
        setPixmap(frames[currentFrame]);
    }
}

void Effect::reset() {
    currentFrame = 0;
    counter = 0;
    finished = false;
    updatePixmap();
}

void Effect::buildBeamEffect() {
    frames.clear();
    mirroredFrames.clear();
    currentFrame = 0;
    counter = 0;
    frameDelay = 4;      // 雷射甩動速度（數字越小越快）
    isLoop = false;      // 甩完一次就自動結束
    finished = false;

    // 載入單張星星圖片
    QPixmap star(":/Project2_Dataset/Image/Kirby_Beam/beam_star.png");
    if (star.isNull()) {
        star = QPixmap(24, 24);
        star.fill(Qt::yellow); // 如果找不到圖片，用黃色方塊防退
    }

    // 將 300x300 畫布的中心點 (150, 150) 設定為魔杖頂端的發射起點 (0,0)
    // 這樣在翻轉畫布時，發射原點絕對不會位移
    qreal centerX = 150;
    qreal centerY = 150;
    setOffset(-centerX, -centerY); 

    // 迴圈渲染 10 個動畫影格
    for (int i = 0; i < 10; ++i) {
        // --- 建立正向（向右甩）影格 ---
        QPixmap canvas(300, 300);
        canvas.fill(Qt::transparent); // 設定透明底色

        QPainter painter(&canvas);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        // 讀取該影格的星星相對座標，並加上中心點平移
        QList<QRectF> rects = getBeamStarRects(i);
        for (const QRectF &r : rects) {
            QRectF canvasRect(r.x() + centerX, r.y() + centerY, r.width(), r.height());
            painter.drawPixmap(canvasRect, star, star.rect());
        }
        painter.end();
        frames.append(canvas);

        // --- 建立反向（向左甩）影格 ---
        // 由於畫布對稱，直接將向右的畫布做水平翻轉，就是完美的向左動畫
        QImage img = canvas.toImage();
        QImage mirroredImg = img.mirrored(true, false);
        mirroredFrames.append(QPixmap::fromImage(mirroredImg));
    }

    updatePixmap();
}

// 📐 6. 10 幀的雷射星星相對座標資料庫（以魔杖頂端 (0,0) 為原點延伸出去）
QList<QRectF> Effect::getBeamStarRects(int frameIndex) const {
    QList<QRectF> rects;
    qreal w = 24, h = 24; // 星星的寬高
    switch(frameIndex) {
        case 0: rects << QRectF(10, -5, w, h)   << QRectF(25, -15, w, h); break;
        case 1: rects << QRectF(20, -10, w, h)  << QRectF(45, -25, w, h)  << QRectF(60, -40, w, h); break;
        case 2: rects << QRectF(30, 0, w, h)    << QRectF(60, -15, w, h)  << QRectF(85, -20, w, h); break;
        case 3: rects << QRectF(40, 15, w, h)   << QRectF(75, 10, w, h)   << QRectF(105, 0, w, h); break;
        case 4: rects << QRectF(45, 30, w, h)   << QRectF(85, 30, w, h)   << QRectF(120, 20, w, h); break;
        case 5: rects << QRectF(40, 45, w, h)   << QRectF(80, 55, w, h)   << QRectF(115, 50, w, h); break;
        case 6: rects << QRectF(30, 60, w, h)   << QRectF(65, 75, w, h)   << QRectF(95, 80, w, h); break;
        case 7: rects << QRectF(20, 70, w, h)   << QRectF(45, 85, w, h)   << QRectF(70, 95, w, h); break;
        case 8: rects << QRectF(10, 65, w, h)   << QRectF(25, 75, w, h)   << QRectF(45, 85, w, h); break;
        case 9: rects << QRectF(5, 50, w, h)    << QRectF(15, 60, w, h); break;
    }
    return rects;
}

// ⭕ 7. 精確碰撞箱複寫：只有畫面上看得到的「星星矩形」會被算入攻擊判定
QPainterPath Effect::shape() const {
    if (currentType == EffectType::Beam && currentFrame >= 0 && currentFrame < frames.size()) {
        QPainterPath path;
        QList<QRectF> rects = getBeamStarRects(currentFrame);
        for (const QRectF &r : rects) {
            if (isMirrored) {
                // 如果是向左鏡像，X 座標做數學對稱反轉 ( x 變成 -x - width )
                path.addRect(QRectF(-r.x() - r.width(), r.y(), r.width(), r.height()));
            } else {
                path.addRect(r);
            }
        }
        return path;
    }
    return QGraphicsPixmapItem::shape(); // 其他非 Beam 特效維持原廠預設的圖片矩形箱
}

