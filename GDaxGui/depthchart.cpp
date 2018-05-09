#include "depthchart.h"

#include "gdaxlib.h"

static constexpr QRgb BidFillColour = qRgb(0,100,0);
static constexpr QRgb BidEdgeColour = qRgb(0,255,0);
static constexpr QRgb AskFillColour = qRgb(139,0,0);
static constexpr QRgb AskEdgeColour = qRgb(255,0,0);

DepthChart::DepthChart(QWidget *parent)
    : QOpenGLWidget(parent)
    , background(Qt::black)
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

    QSize sz = size();

    // calc xy ranges
    // use %age of values around mid price, binary find?

    double xrange = g->AmountMax().getAsDouble();
    double yrange = (g->PriceMax()-g->PriceMin()).getAsDouble();
    //double yOrg = ((priceMax + priceMin)/2).getAsDouble();
    double yOrg = ((bids.rbegin()->first + asks.begin()->first) / 2).getAsDouble();

    // need to recalc maxAmount periodically and also use the values at top\bottom of the view box
    float const fillFactor = 1.0f;
    Decimal amount;
    bool first = true;
    float x = 0, y = 0;
    float width = sz.width() * fillFactor;
    float height = static_cast<float>(sz.height());

    QPen edgePen(QColor(BidEdgeColour), 2);
    painter.setPen(edgePen);
    for (auto bit = bids.rbegin(); bit!=bids.rend(); ++bit)
    {
        auto & bb = *bit;
        amount += bb.second;
        auto px = static_cast<float>((amount.getAsDouble()) / xrange * width);
        double price = bb.first.getAsDouble();
        auto py = static_cast<float>(((yOrg - price) / yrange + 0.5) * height);
        if (first)
        {
            painter.fillRect(QRectF{ QPointF{x, py}, QPointF{px, height} }, BidFillColour);
            painter.drawLine(x, py, px, py);
            x = px;
            y = py;
            first = false;
        }
        else
        {
            painter.fillRect(QRectF{ QPointF{x, py}, QPointF{px, height} }, BidFillColour);
            painter.drawLine(x, y, x, py);
            painter.drawLine(x, py, px, py);
            x = px;
            y = py;

            if (py >= height)
            {
                break;
            }
        }
    }

    amount = 0;
    first = true;
    x = y = 0;
    edgePen = QPen(QColor(AskEdgeColour), 2);
    painter.setPen(edgePen);
    for (auto a : asks)
    {
        amount += a.second;

        auto px = static_cast<float>((amount.getAsDouble()) / xrange * width);
        auto py = static_cast<float>(((yOrg - a.first.getAsDouble()) / yrange + 0.5) * height);

        if (first)
        {
            painter.fillRect(QRectF{ QPointF{x, py}, QPointF{px, 0.f} }, AskFillColour);
            painter.drawLine(x, py, px, py);
            x = px;
            y = py;
            first = false;
        }
        else
        {
            painter.fillRect(QRectF{ QPointF{x, 0}, QPointF{px, y} }, AskFillColour);
            painter.drawLine(x, y, px, y);
            painter.drawLine(px, y, px, py);
            x = px;
            y = py;

            if (py < 0)
            {
                break;
            }
        }
    }
}