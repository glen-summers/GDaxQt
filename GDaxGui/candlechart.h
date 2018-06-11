#ifndef CANDLECHART_H
#define CANDLECHART_H

#include "plot.h"
#include "sma.h"
#include "ema.h"
#include "candle.h"

#include "flogging.h"

#include <QOpenGLWidget>

#include <deque>

struct Tick;
class QPaintEvent;

class CandleChart : public QOpenGLWidget
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<CandleChart>();

    static constexpr int MaxCandleWidth = 32;
    static constexpr int MinCandleWidth = 6;

    QColor background;
    Plot mutable candlePlot;
    std::deque<Candle> candles;
    time_t baseTime, timeDelta;
    QPoint lastDrag;

    // wrap trackHandler
    bool isMouseTracking;
    bool isTouchTracking;
    int tp0Id;
    int tp1Id;
    QRectF originalRect;
    QRectF originalView;

    // wrap
    Sma sma;
    Ema ema;
    void AddMetric(double x, double y)
    {
        sma.Add(x-baseTime, y);
        ema.Add(x-baseTime, y);
    }
    void SetMetric(double x, double y)
    {
        sma.SetCurrentValue(x-baseTime, y);
        ema.SetCurrentValue(x-baseTime, y);
    }
    //wrap
public:
    CandleChart(QWidget * parent = nullptr);

    void SetCandles(std::deque<Candle> newCandles, Granularity granularity);
    void AddTick(const Tick & tick);
    void Heartbeat(const QDateTime & serverTime);
    time_t HitTest(const QPoint & point) const;
    const Candle * FindCandle(time_t time) const;
    void DrawCandleValues(QPainter & painter, const Candle & candle) const;

private:
    bool CheckCandleRollover(const QDateTime & time, const Decimal & price);

    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    bool event(QEvent *event) override;

    void Paint(QPainter & painter) const;
};

#endif // CANDLECHART_H
