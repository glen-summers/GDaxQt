#include "candleoverlay.h"
#include "candlechart.h"

CandleOverlay::CandleOverlay(CandleChart & candleChart)
    : OverlayWidget(&candleChart)
    , candleChart(candleChart)
    , candleTime()
{
    setMouseTracking(true);
}

void CandleOverlay::mouseMoveEvent(QMouseEvent * event)
{
    if (event->buttons() != Qt::MouseButton::NoButton)
    {
        event->ignore();
        return;
    }

    time_t time = candleChart.HitTest(event->pos());
    if (time!=candleTime)
    {
        candleTime=time;
        update();
    }
}

void CandleOverlay::paintEvent(QPaintEvent *)
{
    const Candle * candle = candleChart.FindCandle(candleTime);
    if (candle)
    {
        QPainter painter(this);
        candleChart.DrawCandleValues(painter, *candle);
    }
}
