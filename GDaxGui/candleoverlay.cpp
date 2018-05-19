#include "candleoverlay.h"
#include "candleChart.h"

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
    const Candle * candle = candleChart.candle(candleTime);
    if (candle)
    {
        QPainter painter(this);
        candleChart.drawCandleValues(painter, *candle);
    }
}
