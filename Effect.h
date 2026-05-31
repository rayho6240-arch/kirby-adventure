#ifndef EFFECT_H
#define EFFECT_H

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPixmap>
#include <QString>
#include <QPainterPath> // 👈 處理精確碰撞箱必備

/**
 * @brief Effect 短暫出現的動畫特效類別
 * @details 繼承自 QGraphicsPixmapItem。支援動畫循環與鏡像。
 */
class Effect : public QGraphicsPixmapItem {
public:
    enum class EffectType {
        Inhale,
        Fire,
        Spark,
        Beam
    };

    /**
     * @brief 建構子 (指定特效類型)
     * @param type 特效類型
     * @param parent 父級 QGraphicsItem
     */
    Effect(EffectType type, QGraphicsItem *parent = nullptr);

    /**
     * @brief 建構子 (舊版相容)
     * @param parent 父級 QGraphicsItem
     */
    Effect(QGraphicsItem *parent = nullptr);

    /**
     * @brief 設定特效動畫與載入影格
     * @param pathPattern 路徑規律，如 ":/Project2_Dataset/Image/inhalingBubble/inhale_smoke(%1).png"
     * @param frameCount 總影格數
     * @param delay 影格切換延遲 (單位為幀)
     * @param targetHeight 目標縮放高度限制
     */
    void setEffect(QString pathPattern, int frameCount, int delay, int targetHeight = -1);

    /**
     * @brief 更新動畫，處理影格切換
     */
    void updateAnimation();

    /**
     * @brief 重置動畫狀態
     */
    void reset();

    /**
     * @brief 設定是否鏡像顯示
     * @param mirrored true 表示鏡像，false 表示原始方向
     */
    void setMirror(bool mirrored);

    /**
     * @brief 設定是否循環播放
     */
    void setLoop(bool loop);

    /**
     * @brief 取得是否循環播放
     */
    bool getLoop() const;

    /**
     * @brief 檢查動畫是否已播放完畢
     * @note 非循環動畫播放完畢時，此函式回傳 true
     */
    bool isFinished() const;

    // 👈 複寫碰撞箱：雖然圖片裡有卡比身體，但我們讓碰撞箱「只有星星」會造成傷害！
    QPainterPath shape() const override;

private:
    /**
     * @brief 根據當前影格與鏡像狀態更新顯示的圖片
     */
    void updatePixmap();

    // 👈 3. 新增：雷射專用的圖層動態合成與座標資料庫函式
    void buildBeamEffect(); 
    QList<QRectF> getBeamStarRects(int frameIndex) const;

    QList<QPixmap> frames;          ///< 原始影格列表 (影格緩存)
    QList<QPixmap> mirroredFrames;  ///< 鏡像影格列表
    
    int currentFrame;               ///< 當前影格索引
    int frameDelay;                 ///< 影格切換延遲
    int counter;                    ///< 幀數計數器
    bool isLoop;                    ///< 是否循環播放
    bool isMirrored;                ///< 是否鏡像顯示
    bool finished;                  ///< 非循環動畫是否播放結束的標記

    EffectType currentType; // 👈 紀錄當前的特效類型
};

#endif // EFFECT_H
