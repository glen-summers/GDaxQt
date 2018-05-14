#include "candlechart.h"

#include "sma.h"

#include <QApplication>

CandleChart::CandleChart(QWidget *parent)
    : QOpenGLWidget(parent)
    , background(QApplication::palette().color(QPalette::Base))
    , candlePlot(10, true, true)
{
}

void CandleChart::SetCandles(std::deque<Candle> forkHandles)
{
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
    double maxTime;
    candles = std::move(forkHandles);
    if (!candles.empty())
    {
        maxTime = candles.begin()->startTime + timeDelta /2;
    }
    else
    {
        maxTime = time(nullptr);
    }
    double minTime = maxTime - initDisplay;

    CandleLess less{};
    for (auto it = std::lower_bound(candles.rbegin(), candles.rend(), (time_t)minTime, less); it!=candles.rend(); ++it)
    {
        max = std::max(max, it->highestPrice.getAsDouble());
        min = std::min(min, it->lowestPrice.getAsDouble());
    }

    Sma sma1(smaPath1, 15);
    Sma sma2(smaPath2, 50);
    for (auto it = candles.rbegin(); it!=candles.rend(); ++it)
    {
        auto endTime = static_cast<double>(it->startTime + timeDelta);
        double close = it->closingPrice.getAsDouble();
        sma1.Add(endTime, close);
        sma2.Add(endTime, close);
    }

    QRectF view(minTime, min, maxTime-minTime, max-min);
    double margin = view.height()*0.1;
    view.adjust(0, -margin, 0, margin);
    candlePlot.SetView(view);
}

void CandleChart::wheelEvent(QWheelEvent * event)
{
    QPointF pos = event->posF();
    double delta = event->angleDelta().y()/1200.; // 120|15degs per click
    double candleWidth = candlePlot.ScaleToScreen({(double)timeDelta, 0}).x();

    if (pos.x() > candlePlot.Inner().right())
    {
        candlePlot.ZoomY(pos, delta);
    }
    else if ((delta<0 && candleWidth > MinCandleWidth) || (delta>0 && candleWidth < MaxCandleWidth))
    {
        candlePlot.ZoomX(pos, delta);
    }
    event->accept();
    update();
}

void CandleChart::mousePressEvent(QMouseEvent *event)
{
    lastDrag = event->pos();
    qApp->setOverrideCursor(Qt::ClosedHandCursor);
    setMouseTracking(true);
}

void CandleChart::mouseMoveEvent(QMouseEvent *event)
{
    auto delta = (event->pos() - lastDrag);
    lastDrag = event->pos();
    candlePlot.Pan(delta.x(), delta.y());
    update();
}

void CandleChart::mouseReleaseEvent(QMouseEvent *)
{
    qApp->restoreOverrideCursor();
    setMouseTracking(false);
}

void CandleChart::Paint(QPainter & painter) const
{
    // todo, feed in new tick values
    // add ema
    // init 1d hourly

    if (candles.empty())
    {
        return;
    }

    candlePlot.SetRect(rect()); // or on event?
    candlePlot.StartInner(painter);

    //time_t startTime = time(nullptr); // or lastUPdateTime / manual scroll value
    // if auto scroll at new time seg

    // mode = 1hr... others
    //double yRange = (max-min);
    //double wedgie = 0.3*timeDelta*xScale;

    // to cfg
    QPen penUp(QColor(qRgb(0,255,0)), 1);
    QBrush brushUp(background);
    QPen penDown(QColor(qRgb(255,0,0)), 1);
    QBrush brushDown(QColor(qRgb(255,0,0)));
    penUp.setCosmetic(true);
    penDown.setCosmetic(true);

    painter.save();
    candlePlot.ApplyViewTransform(painter);

    CandleLess less;
    QRectF v = candlePlot.View();
    auto start = (time_t)v.left() - timeDelta/2;
    auto finish = (time_t)v.right() + timeDelta/2;
    for (auto it = std::lower_bound(candles.rbegin(), candles.rend(), start, less),
         end = std::upper_bound(candles.rbegin(), candles.rend(), finish, less);
         it!=end; ++it)
    {
        const auto & c = *it;

        QBrush * fillBrush = nullptr;
        if (c.closingPrice > c.openingPrice)
        {
            painter.setPen(penUp);
            painter.setBrush(brushUp);
        }
        else if (c.closingPrice < c.openingPrice)
        {
            painter.setPen(penDown);
            fillBrush = &brushDown;
        }
        else
        {
            continue;
        }

        candlePlot.DrawCandle(painter, c.startTime, c.startTime + timeDelta
                              , c.lowestPrice.getAsDouble()
                              , c.highestPrice.getAsDouble()
                              , c.openingPrice.getAsDouble()
                              , c.closingPrice.getAsDouble(),
                              fillBrush);
    }

    QPen smaPen(QColor(qRgb(255,255,0)), 1.5);
    smaPen.setCosmetic(true);
    painter.setPen(smaPen);
    candlePlot.DrawPath(painter, smaPath1);
    smaPen.setColor(qRgb(0,255,255));
    painter.setPen(smaPen);
    candlePlot.DrawPath(painter, smaPath2);

    painter.restore();
    candlePlot.EndInner(painter);
    candlePlot.DrawTimeAxis(painter);
    candlePlot.DrawYAxis(painter, 0, false);
    candlePlot.DrawYAxis(painter, 1, true);
}
