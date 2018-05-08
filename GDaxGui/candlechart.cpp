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
    // todo, feed in new tick values
    // add ema

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

    //time_t startTime = candles.rbegin()->startTime;
    //time_t endTime = candles.begin()->startTime + 60*60;

    time_t startTime = time(nullptr);

    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
    for (const auto & c : candles)
    {
        max = std::max(max, c.highestPrice.getAsDouble());
        min = std::min(min, c.lowestPrice.getAsDouble());
    }

    // mode = 1hr
    time_t timeDelta = 60*60; //1d init, max = endTime - startTime;
    float xScale = 500.f/24/60/60; // init 500px per day
    double yRange = (max-min);
    double wedgie = 0.3*timeDelta*xScale;

    int width = this->width();
    int height = this->height();

    QPen penUp(QColor(qRgb(0,255,0)), 2);
    QBrush brushUp(QColor(qRgb(0,0,0)));
    QPen penDown(QColor(qRgb(255,0,0)), 2);
    QBrush brushDown(QColor(qRgb(255,0,0)));

    for (const auto & c : candles) // goes back in time...
    {
        float x = width - (startTime - c.startTime) * xScale; // from right!
        auto y0 = static_cast<float>(((max-c.lowestPrice.getAsDouble()) / yRange) * height);
        auto y1 = static_cast<float>(((max-c.highestPrice.getAsDouble()) / yRange) * height);
        auto yy0 = static_cast<float>(((max-c.openingPrice.getAsDouble()) / yRange) * height);
        auto yy1 = static_cast<float>(((max-c.closingPrice.getAsDouble()) / yRange) * height);

        if (c.closingPrice > c.openingPrice)
        {
            painter.setPen(penUp);
            painter.setBrush(brushUp);
        }
        else if (c.closingPrice < c.openingPrice)
        {
            painter.setPen(penDown);
            painter.setBrush(brushDown);
        }
        else
        {
            continue;
        }

        painter.drawLine(QPointF(x, y0), QPointF(x, y1));
        // if xscale > ...
        painter.drawRect(QRectF(QPointF(x-wedgie, yy0), QPointF(x+wedgie, yy1)));
    }

}
