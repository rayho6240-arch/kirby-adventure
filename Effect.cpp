#include "Effect.h"
#include <QImage>

Effect::Effect(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent),
      currentFrame(0),
      frameDelay(0),
      counter(0),
      isLoop(false),
      isMirrored(false),
      finished(false) {
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
