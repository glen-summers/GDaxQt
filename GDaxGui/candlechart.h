#ifndef CANDLECHART_H
#define CANDLECHART_H

#include "plot.h"
#include "sma.h"
#include "ema.h"
#include "candle.h"
#include "touchhandler.h"

#include "flogging.h"

#include <QOpenGLWidget>

#include <deque>

struct Tick;
class QPaintEvent;

class CandleChart : public QOpenGLWidget, public Touchee
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<CandleChart>();

    static constexpr int MaxCandleWidth = 32;
    static constexpr int MinCandleWidth = 6;

    QColor background;
    Plot mutable candlePlot;
    std::deque<Candle> candles;
    time_t baseTime, timeDelta;
    TouchHandler touchHandler;

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

    // touchee
    const QRectF & View() const override { return candlePlot.View(); }
    void SetView(const QRectF & view) const override { candlePlot.SetView(view);}
    void Pan(double, double) override;
    void Scale(const QPointF & p, double xScale, double yScale) override;
};

#endif // CANDLECHART_H
