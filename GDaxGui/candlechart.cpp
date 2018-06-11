#include "candlechart.h"

#include "tick.h"
#include "qfmt.h"

#include <QApplication>
#include <QPainter>
#include <QPaintEvent>

#include <cassert>

CandleChart::CandleChart(QWidget *parent)
    : QOpenGLWidget(parent)
    , background(QApplication::palette().color(QPalette::Base))
    , candlePlot(10, true, true)
    , baseTime()
    , timeDelta()
    , lastDrag()
    , isMouseTracking()
    , isTouchTracking()
    , tp0Id()
    , tp1Id()
    , sma(15)
    , ema(15)
{
}

void CandleChart::SetCandles(std::deque<Candle> forkHandles, Granularity granularity)
{
    Flog::ScopeLog s(log, Flog::Level::Info, "SetCandles");

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

    sma.Reset();
    ema.Reset();
    for (auto it = candles.rbegin(); it!=candles.rend(); ++it)
    {
        double close = it->closingPrice.getAsDouble();
        AddMetric(it->startTime, close);
    }

    QRectF view(minTime-baseTime, min, maxTime-minTime, max-min);
    double margin = view.height()*0.1;
    view.adjust(0, -margin, 0, margin);
    candlePlot.SetView(view);
    update();
}

void CandleChart::AddTick(const Tick & tick)
{
    if (candles.empty())
    {
        // tick before candles arrive, could queue and apply later if compare seqNos
        return;
    }

    CheckCandleRollover(tick.time, candles.front().closingPrice);
    auto & candle = candles.front();
    candle.highestPrice = std::max(candle.highestPrice, tick.price);
    candle.lowestPrice= std::min(candle.lowestPrice, tick.price);
    candle.closingPrice = tick.price;
    SetMetric(candle.startTime, tick.price.getAsDouble());
    update();
}

void CandleChart::Heartbeat(const QDateTime & serverTime)
{
    if (!candles.empty() && CheckCandleRollover(serverTime, candles.front().closingPrice))
    {
        update();
    }
}

const Candle * CandleChart::FindCandle(time_t time) const
{
    auto it = std::lower_bound(candles.rbegin(), candles.rend(), time, CandleLess{});
    return it == candles.rend() || it->startTime > time + timeDelta ? nullptr : &*it;
}

time_t CandleChart::HitTest(const QPoint & point) const
{
    return baseTime + candlePlot.MapToView(point).x() - timeDelta /2;
}

void CandleChart::DrawCandleValues(QPainter & painter, const Candle & candle) const
{
    auto text = QString("%1 O:%2 H:%3 L:%4 C:%5") // +% change, need previous candle
            .arg(QDateTime::fromSecsSinceEpoch(candle.startTime).toString("MMM dd yyyy hh:mm"))
            .arg(candle.openingPrice.getAsDouble(), 0, 'f', 2)
            .arg(candle.highestPrice.getAsDouble(), 0, 'f', 2)
            .arg(candle.lowestPrice.getAsDouble(), 0, 'f', 2)
            .arg(candle.closingPrice.getAsDouble(), 0, 'f', 2);

    auto inner = candlePlot.Inner();
    painter.setClipRect(inner);
    // draw alpha rect for text visibility
    painter.drawText(candlePlot.Inner().topLeft()+QPointF{5,15}, text);
    painter.setClipping(false);
}

bool CandleChart::CheckCandleRollover(const QDateTime & dateTime, const Decimal & price)
{
    bool updated = false;
    time_t time = dateTime.toSecsSinceEpoch();
    time_t endTime = candles.front().startTime + timeDelta;
    if (time >= endTime) // while in case multiple gaps?
    {
        candles.push_front(Candle{endTime, price, price, price, price, {}}); // +amount
        AddMetric(endTime, price.getAsDouble());

        // scoll by timeSeg, always or only if not manually scrolled?
        auto view = candlePlot.View();
        view.adjust(timeDelta, 0, timeDelta, 0);
        candlePlot.SetView(view);
        updated = true;
    }
    return updated;
}

void CandleChart::paintEvent(QPaintEvent *event)
{
    Flog::ScopeLog s(log, Flog::Level::Debug, "paintEvent", "--");

    QPainter painter;
    painter.begin(this);
    painter.fillRect(event->rect(), background);
    painter.setRenderHint(QPainter::Antialiasing);
    //painter.setRenderHint(QPainter::HighQualityAntialiasing);
    Paint(painter);
    painter.end();
}

void CandleChart::wheelEvent(QWheelEvent * event)
{
    QPointF pos = event->posF();
    double delta = event->angleDelta().y()/1200.; // 120|15degs per click = +-0.1 per wheel tick
    double candleWidth = candlePlot.ScaleToScreen({(double)timeDelta, 0}).x(); // cache

    auto scale = delta>=0 ? 1+delta : 1 / (1-delta);

    if (pos.x() > candlePlot.Inner().right())
    {
        candlePlot.ZoomY(pos, scale);
    }
    else if ((delta<0 && candleWidth > MinCandleWidth) || (delta>0 && candleWidth < MaxCandleWidth))
    {
        candlePlot.ZoomX(pos, scale);
    }
    event->accept();
    update();
}

void CandleChart::mousePressEvent(QMouseEvent *event)
{
    lastDrag = event->pos();
    qApp->setOverrideCursor(Qt::ClosedHandCursor);
    isMouseTracking = true;
    event->accept();
}

void CandleChart::mouseMoveEvent(QMouseEvent *event)
{
    if (isTouchTracking)
    {
        // avoid double pan
        return;
    }

    auto delta = event->pos() - lastDrag;
    log.Info("MouseMove {0}, [{1}]", event->pos(), delta);
    lastDrag = event->pos();
    candlePlot.Pan(delta.x(), delta.y());
    update();
    event->accept();
}

void CandleChart::mouseReleaseEvent(QMouseEvent * event)
{
    qApp->restoreOverrideCursor();
    event->accept();
    isMouseTracking = false;
}

bool CandleChart::event(QEvent * event)
{
    // ignore size() ==1 as is covered by mouse, but what it isnt?
    // AA_SynthesizeTouchForUnhandledMouseEvents
    // if have mouse -> touch then just do touch?
    // when go to 2 pointers, mouse is still getting one of them!
    // use aaflag and just use touch? - has bug mouse->touch events dont have dpi scaling applied

    switch (event->type())
    {
        case QEvent::TouchBegin:
        {
            event->accept();
            return true; // same as accept?
        }

        case QEvent::TouchCancel:
            log.Info("TouchCancel");
            // set to ignore
            break;

        case QEvent::TouchEnd:
        {
            log.Info("TouchEnd");
            isTouchTracking  = false;
            event->accept();
            break;
        }

        case QEvent::TouchUpdate:
        {
            QTouchEvent * touchEvent = (QTouchEvent *)event;
            auto tps = touchEvent->touchPoints();

            // AA_SynthesizeTouchForUnhandledMouseEvents
            // if (tps.size()==1 && !isMouseTracking)...

            if (tps.size() == 2)
            {
                auto & tp0 = tps[0];
                auto & tp1 = tps[1];

                if (isTouchTracking && (tp0Id != tp0.id() || tp1Id != tp1.id()))
                {
                    isTouchTracking  = false;
                    log.Info("new touchPoint ids");
                }

                auto rc = QRectF(tp0.pos(), tp1.pos()).normalized();
                if (!isTouchTracking)
                {
                    tp0Id = tp0.id();
                    tp1Id = tp1.id();

                    originalRect = rc;
                    originalView = candlePlot.View();
                    event->accept();
                    log.Info("start touchTracking rect:{0} view:{1}, id0:{2}, id1:{3}", rc, candlePlot.View(), tp0Id, tp1Id);
                    return isTouchTracking = true;
                }

                candlePlot.SetView(originalView);
                auto delta = rc.center() - originalRect.center();
                candlePlot.Pan(delta.x(), delta.y());

                auto origLength = QLineF(originalRect.topLeft(), originalRect.bottomRight()).length(); //0? cache?
                auto length = QLineF(rc.topLeft(), rc.bottomRight()).length();
                auto scale = length/origLength - 1;
                auto t2 = atan2(rc.height(), rc.width());
                auto scaleX = 1 + scale * cos(t2);
                auto scaleY = 1 + scale * sin(t2);
                auto minScale = MinCandleWidth * candlePlot.View().width() / timeDelta / candlePlot.Inner().width();
                auto maxScale = MaxCandleWidth * candlePlot.View().width() / timeDelta / candlePlot.Inner().width();
                if (scaleX < minScale) scaleX = minScale;
                if (scaleX > maxScale) scaleX = maxScale;
                candlePlot.ZoomX(rc.center(), scaleX);
                candlePlot.ZoomY(rc.center(), scaleY);
                update();

                log.Info("touchTrack rect:{0} view:{1}, sx:{2}, sy:{3}, id0:{4}, id1:{5}", rc, candlePlot.View(), scaleX, scaleY,
                         tps[0].id(), tps[1].id());
            }

            if (isTouchTracking && tps.size() != 2)
            {
                isTouchTracking  = false;
                log.Info("touchTracking  = false;");
            }
            event->accept();
            return true;
        }

        default:;
    }
    return QOpenGLWidget::event(event);
}

void CandleChart::Paint(QPainter & painter) const
{
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
    for (auto end = candles.rend(), it = std::lower_bound(candles.rbegin(), end, baseTime + start, less);
         it != end && it->startTime < baseTime + finish; ++it)
    {
        const auto & c = *it;
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
    candlePlot.DrawPath(painter, sma.Path());
    smaPen.setColor(qRgb(0,255,255));
    painter.setPen(smaPen);
    candlePlot.DrawPath(painter, ema.Path());

    painter.restore();
    candlePlot.EndInner(painter);
    candlePlot.DrawTimeAxis(painter, baseTime);
    candlePlot.DrawYAxis(painter, 0, false);
    candlePlot.DrawYAxis(painter, 1, true);
}
