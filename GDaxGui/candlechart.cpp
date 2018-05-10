#include "candlechart.h"

#include <QApplication>

CandleChart::CandleChart(QWidget *parent)
    : QOpenGLWidget(parent)
    , background(QApplication::palette().color(QPalette::Base))
    , candlePlot(10)
{
}

void CandleChart::setCandles(std::vector<Candle> forkHandles)
{
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
    double minTime, maxTime;
    candles = std::move(forkHandles);
    if (!candles.empty())
    {
        minTime = candles.rbegin()->startTime - timeDelta /2;
        maxTime = candles.begin()->startTime + timeDelta /2;
    }
    else
    {
        minTime = maxTime = time(nullptr);
    }

    if (minTime < maxTime - initDisplay)
    {
        minTime = maxTime - initDisplay;
    }

    for (const auto & c : candles)
    {
        max = std::max(max, c.highestPrice.getAsDouble());
        min = std::min(min, c.lowestPrice.getAsDouble());
        if (c.startTime<minTime)
        {
            break;
        }
    }

    QRectF view(minTime, min, maxTime-minTime, max-min);
    double  margin = view.height()*0.1;
    view.adjust(0, -margin, 0, margin);
    candlePlot.setView(view);
}

void CandleChart::paint(QPainter & painter) const
{
    // todo, feed in new tick values
    // add ema
    // init 1d hourly

    if (candles.empty())
    {
        return;
    }

    candlePlot.setRect(rect()); // or on event?
    candlePlot.startInner(painter);

    //time_t startTime = time(nullptr); // or lastUPdateTime / manual scroll value

    // mode = 1hr... others
    //double yRange = (max-min);
    //double wedgie = 0.3*timeDelta*xScale;

    // to cfg
    QPen penUp(QColor(qRgb(0,255,0)), 1);
    QBrush brushUp(background);
    QPen penDown(QColor(qRgb(255,0,0)), 1);
    QBrush brushDown(QColor(qRgb(255,0,0)));

    // binary find range...
    for (const auto & c : candles) // goes back in time...
    {
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

        candlePlot.drawCandle(painter, c.startTime, c.startTime + timeDelta
                              , c.lowestPrice.getAsDouble()
                              , c.highestPrice.getAsDouble()
                              , c.openingPrice.getAsDouble()
                              , c.closingPrice.getAsDouble());
    }

    candlePlot.endInner(painter);
    candlePlot.drawTimeAxis(painter);
}
