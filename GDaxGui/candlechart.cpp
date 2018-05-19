#include "candlechart.h"

#include "sma.h"
#include "ema.h"

#include <QApplication>

CandleChart::CandleChart(QWidget *parent)
    : QOpenGLWidget(parent)
    , background(QApplication::palette().color(QPalette::Base))
    , candlePlot(10, true, true)
    , baseTime()
    , timeDelta()
    , lastDrag()
{
}

void CandleChart::SetCandles(std::deque<Candle> forkHandles, Granularity granularity)
{
    timeDelta = (unsigned int)granularity;
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
    double maxTime, minTime;

    // now using baseTime as an offset to avoid float(32) graphics innaccuracy
    // when using a transform origin at unix epoch ~= 1,526,486,078 seconds
    candles = std::move(forkHandles);
    if (!candles.empty())
    {
        maxTime = candles.begin()->startTime + timeDelta /2;
        baseTime = candles.rbegin()->startTime;
    }
    else
    {
        baseTime = maxTime = time(nullptr);
    }

    minTime = maxTime - timeDelta*100; // param, validate\calc for maxCandleWidth?

    CandleLess less{};
    for (auto it = std::lower_bound(candles.rbegin(), candles.rend(), (time_t)minTime, less); it!=candles.rend(); ++it)
    {
        max = std::max(max, it->highestPrice.getAsDouble());
        min = std::min(min, it->lowestPrice.getAsDouble());
    }

    Sma sma(smaPath, 15);
    Ema ema(emaPath, 15);
    for (auto it = candles.rbegin(); it!=candles.rend(); ++it)
    {
        auto startTime = static_cast<double>(it->startTime - baseTime);
        double close = it->closingPrice.getAsDouble();
        sma.Add(startTime, close);
        ema.Add(startTime, close);
    }

    QRectF view(minTime-baseTime, min, maxTime-minTime, max-min);
    double margin = view.height()*0.1;
    view.adjust(0, -margin, 0, margin);
    candlePlot.SetView(view);

    update();
}

const Candle * CandleChart::candle(time_t time) const
{
    auto it = std::upper_bound(candles.rbegin(), candles.rend(), time, CandleLess{});
    return (it == candles.rend()) ? nullptr : &*it;
}

time_t CandleChart::HitTest(const QPoint & point) const
{
    return baseTime + candlePlot.MapToView(point).x() - timeDelta /2;
}

void CandleChart::drawCandleValues(QPainter & painter, const Candle & candle) const
{
    auto text = QString("%1 O:%2 H:%3 L:%4 C:%5") // +% change
            .arg(QDateTime::fromSecsSinceEpoch(candle.startTime).toString("MMM dd yyyy hh:mm"))
            .arg(candle.openingPrice.getAsDouble(), 0, 'f', 2)
            .arg(candle.highestPrice.getAsDouble(), 0, 'f', 2)
            .arg(candle.lowestPrice.getAsDouble(), 0, 'f', 2)
            .arg(candle.closingPrice.getAsDouble(), 0, 'f', 2);

    painter.drawText(candlePlot.Inner(), text, Qt::AlignLeft | Qt::AlignTop);
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

    if (candles.empty())
    {
        return;
    }

    candlePlot.SetRect(rect()); // or on event?
    candlePlot.StartInner(painter);

    //time_t startTime = time(nullptr); // or lastUPdateTime / manual scroll value
    // if auto scroll at new time seg

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
    for (auto it = std::lower_bound(candles.rbegin(), candles.rend(), baseTime + start, less),
         end = std::upper_bound(candles.rbegin(), candles.rend(), baseTime + finish, less);
         it!=end; ++it)
    {
        const auto & c = *it; // getting non decremental exception here, *it calls --

        if (c.closingPrice >= c.openingPrice)
        {
            painter.setPen(penUp);
            painter.setBrush(brushUp);
        }
        else if (c.closingPrice < c.openingPrice)
        {
            painter.setPen(penDown);
            painter.setBrush(brushDown);
        }

        auto startTime = c.startTime - baseTime;
        candlePlot.DrawCandle(painter, startTime, startTime + timeDelta
                              , c.lowestPrice.getAsDouble()
                              , c.highestPrice.getAsDouble()
                              , c.openingPrice.getAsDouble()
                              , c.closingPrice.getAsDouble());
    }

    painter.setBrush({});
    QPen smaPen(QColor(qRgb(255,255,0)), 1.5);
    smaPen.setCosmetic(true);
    painter.setPen(smaPen);
    candlePlot.DrawPath(painter, smaPath);
    smaPen.setColor(qRgb(0,255,255));
    painter.setPen(smaPen);
    candlePlot.DrawPath(painter, emaPath);

    painter.restore();
    candlePlot.EndInner(painter);
    candlePlot.DrawTimeAxis(painter, baseTime);
    candlePlot.DrawYAxis(painter, 0, false);
    candlePlot.DrawYAxis(painter, 1, true);
}
