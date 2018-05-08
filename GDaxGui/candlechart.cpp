#include "candlechart.h"

CandleChart::CandleChart(QWidget *parent)
    : QOpenGLWidget(parent)
    , background(Qt::black)
{
}

void CandleChart::setCandles(std::vector<Candle> candles)
{
    this->candles = std::move(candles);
}

void CandleChart::paint(QPainter & painter) const
{
    /*
    time_t startTime;
    Decimal lowestPrice;
    Decimal highestPrice;
    Decimal openingPrice;
    Decimal closingPrice;
    Decimal volume;*/

    // init 1d hourly

    if (candles.empty())
    {
        return;
    }

    time_t startTime = candles.rbegin()->startTime;
    time_t endTime = candles.begin()->startTime + 3600;

    double min = std::numeric_limits<double >::max();
    double max = std::numeric_limits<double >::lowest();
    for (const auto & c : candles)
    {
        max = std::max(max, c.highestPrice.getAsDouble());
        min = std::min(min, c.lowestPrice.getAsDouble());
    }

    time_t timeRange = endTime - startTime;
    double yRange = (max-min);

    float width = this->width();
    float height = this->height();

    QPen penUp(QColor(qRgb(0,255,0)), 2);
    QPen penDown(QColor(qRgb(255,0,0)), 2);

    for (const auto & c : candles) // gies back in time...
    {
        float x = width * (c.startTime - startTime) / timeRange;
        auto y0 = static_cast<float>(((max-c.lowestPrice.getAsDouble()) / yRange) * height);
        auto y1 = static_cast<float>(((max-c.highestPrice.getAsDouble()) / yRange) * height);

        if (c.closingPrice > c.openingPrice)
        {
            painter.setPen(penUp);
            painter.drawLine(QPointF(x, y0), QPointF(x, y1));
        }
        else if (c.closingPrice < c.openingPrice)
        {
            painter.setPen(penDown);
            painter.drawLine(QPointF(x, y0), QPointF(x, y1));
        }

// if xscale > ...
//        auto yy0 = static_cast<float>(((c.openingPrice - min) / yRange).getAsDouble() * height);
//        auto yy1 = static_cast<float>(((c.closingPrice - min) / yRange).getAsDouble() * height);
//        painter.drawRect(QRectF(x-xDelta/2, yy0, x+xDelta/2, yy1));
    }

}
