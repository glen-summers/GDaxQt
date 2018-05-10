#include "depthchart.h"

#include "gdaxlib.h"

static constexpr QRgb BidFillColour = qRgb(0,100,0);
static constexpr QRgb BidEdgeColour = qRgb(0,255,0);
static constexpr QRgb AskFillColour = qRgb(139,0,0);
static constexpr QRgb AskEdgeColour = qRgb(255,0,0);

#include <QApplication>

DepthChart::DepthChart(QWidget *parent)
    : QOpenGLWidget(parent)
    , background(QApplication::palette().color(QPalette::Base))
    , depthPlot(10, false)
    , g()
{
    setAutoFillBackground(false);
}

void DepthChart::paint(QPainter & painter) const
{
    if (!g)
    {
        return;
    }

    const auto & bids = g->Bids();
    const auto & asks = g->Asks();

    if (bids.empty() || asks.empty())
    {
        return;
    }

    depthPlot.setRect(rect()); // or on event?
    depthPlot.startInner(painter);

     // calc xy ranges
    // use %age of values around mid price, binary find?

    double xrange = g->AmountMax().getAsDouble();
    double yrange = (g->PriceMax()-g->PriceMin()).getAsDouble();
    //double yOrg = ((priceMax + priceMin)/2).getAsDouble();
    double yOrg = ((bids.rbegin()->first + asks.begin()->first) / 2).getAsDouble();

    // need to recalc maxAmount periodically and also use the values at top\bottom of the view box
    double const fillFactor = 1.0;
    Decimal amount;
    bool first = true;

    QRectF rect = depthPlot.getInner();
    double ox = rect.x(), oy = rect.y();
    double x = ox, y = oy;
    double width = rect.width() * fillFactor;
    double height = rect.height();

    QPen edgePen(QColor(BidEdgeColour), 2);
    painter.setPen(edgePen);
    for (auto bit = bids.rbegin(); bit!=bids.rend(); ++bit)
    {
        auto & bb = *bit;
        amount += bb.second;
        double px = ox + (amount.getAsDouble() / xrange * width);
        double price = bb.first.getAsDouble();
        double py = oy +((yOrg - price) / yrange + 0.5) * height;
        if (first)
        {
            painter.fillRect(QRectF{ QPointF{x, py}, QPointF{px, oy + height} }, BidFillColour);
            painter.drawLine(x, py, px, py);
            x = px;
            y = py;
            first = false;
        }
        else
        {
            painter.fillRect(QRectF{ QPointF{x, py}, QPointF{px, oy + height} }, BidFillColour);
            painter.drawLine(x, y, x, py);
            painter.drawLine(x, py, px, py);
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
    x = ox;
    y = oy;
    edgePen = QPen(QColor(AskEdgeColour), 2);
    painter.setPen(edgePen);
    for (auto a : asks)
    {
        amount += a.second;

        double px = ox + (amount.getAsDouble()) / xrange * width;
        double py = oy + ((yOrg - a.first.getAsDouble()) / yrange + 0.5) * height;

        if (first)
        {
            painter.fillRect(QRectF{ QPointF{x, py}, QPointF{px, oy} }, AskFillColour);
            painter.drawLine(x, py, px, py);
            x = px;
            y = py;
            first = false;
        }
        else
        {
            painter.fillRect(QRectF{ QPointF{x, oy}, QPointF{px, y} }, AskFillColour);
            painter.drawLine(x, y, px, y);
            painter.drawLine(px, y, px, py);
            x = px;
            y = py;

            if (py - oy < 0)
            {
                break;
            }
        }
    }

    depthPlot.endInner(painter);
}
