#include "depthchart.h"

#include "orderbook.h"

static constexpr QRgb BidFillColour = qRgb(0,100,0);
static constexpr QRgb BidEdgeColour = qRgb(0,255,0);
static constexpr QRgb AskFillColour = qRgb(139,0,0);
static constexpr QRgb AskEdgeColour = qRgb(255,0,0);

#include <QApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QMutex>

DepthChart::DepthChart(QWidget *parent)
    : QOpenGLWidget(parent)
    , background(QApplication::palette().color(QPalette::Base))
    , depthPlot(10, false, true)
    , gdl()
{
    setAutoFillBackground(false);
}

void DepthChart::paintEvent(QPaintEvent *event)
{
    Flog::ScopeLog s(log, Flog::Level::Debug, "paintEvent", "--");

    QPainter painter;
    painter.begin(this);
    painter.fillRect(event->rect(), background);
    painter.setRenderHint(QPainter::Antialiasing);
    Paint(painter);
    painter.end();
}

void DepthChart::Paint(QPainter & painter) const
{
    if (!gdl)
    {
        return;
    }

    // lock orderbook, move\improve impl
    const auto & orderBook = gdl->Orders();
    QMutexLocker lock(&const_cast<QMutex&>(orderBook.Mutex()));

    const auto & bids = orderBook.Bids();
    const auto & asks = orderBook.Asks();

    if (bids.empty() || asks.empty())
    {
        return;
    }

    // try using a Path object and scaling after to fit?

    const int perMille = 10;
    auto mid = orderBook.MidPrice();
    auto seekRange = mid * perMille / 1000;
    auto lo = mid - seekRange;
    auto hi = mid + seekRange;

    double xRange = orderBook.SeekAmount(lo, hi).getAsDouble();
    double yRange = seekRange.getAsDouble() * 2;
    double yOrg = mid.getAsDouble();
    depthPlot.SetView(QRectF{QPointF{0, lo.getAsDouble()}, QPointF{xRange, hi.getAsDouble()}});

    depthPlot.SetRect(rect()); // or on event?
    depthPlot.StartInner(painter);

    double const fillFactor = 0.75;
    Decimal amount;
    bool first = true;

    QRectF rect = depthPlot.Inner();
    double width = rect.width() * fillFactor;
    double height = rect.height();
    double ox = rect.right(), oy = rect.y();
    double x = ox, y = oy;

    QPen edgePen(QColor(BidEdgeColour), 1.5);
    painter.setPen(edgePen);
    for (auto bit = bids.rbegin(); bit!=bids.rend(); ++bit)
    {
        auto & bb = *bit;
        amount += bb.second;
        double px = ox - (amount.getAsDouble() / xRange * width);
        double price = bb.first.getAsDouble();
        double py = oy +((yOrg - price) / yRange + 0.5) * height;
        if (first)
        {
            painter.fillRect(QRectF{ QPointF{px, py}, QPointF{x, oy + height} }, BidFillColour);
            painter.drawLine(px, py, x, py);
            x = px;
            y = py;
            first = false;
        }
        else
        {
            painter.fillRect(QRectF{ QPointF{px, py}, QPointF{x, oy + height} }, BidFillColour);
            painter.drawLine(x, y, x, py);
            painter.drawLine(px, py, x, py);
            x = px;
            y = py;

            if (py - oy > height)
            {
                break;
            }
        }
    }

    amount = 0;
    first = true;
    x = ox, y = oy;
    edgePen = QPen(QColor(AskEdgeColour), 1.5);
    painter.setPen(edgePen);
    for (auto a : asks)
    {
        amount += a.second;

        double px = ox - (amount.getAsDouble()) / xRange * width;
        double py = oy + ((yOrg - a.first.getAsDouble()) / yRange + 0.5) * height;

        if (first)
        {
            painter.fillRect(QRectF{ QPointF{px, py}, QPointF{x, oy} }, AskFillColour);
            painter.drawLine(px, py, x, py);
            x = px;
            y = py;
            first = false;
        }
        else
        {
            painter.fillRect(QRectF{ QPointF{px, oy}, QPointF{x, y} }, AskFillColour);
            painter.drawLine(px, y, x, y);
            painter.drawLine(px, y, px, py);
            x = px;
            y = py;

            if (py - oy < 0)
            {
                break;
            }
        }
    }

    depthPlot.EndInner(painter);

    depthPlot.DrawYAxis(painter, 0, false);
    depthPlot.DrawYAxis(painter, 1, true);
}
