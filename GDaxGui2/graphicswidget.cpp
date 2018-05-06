#include "graphicswidget.h"

static constexpr QRgb BidFillColour = qRgb(0,100,0);
static constexpr QRgb BidEdgeColour = qRgb(0,255,0);
static constexpr QRgb AskFillColour = qRgb(139,0,0);
static constexpr QRgb AskEdgeColour = qRgb(255,0,0);

GraphicsWidget::GraphicsWidget(const GDaxLib & g, QWidget *parent)
    : QOpenGLWidget(parent)
    , g(g)
{
    //setFixedSize(200, 200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAutoFillBackground(false);

    background = QBrush(QColor(64, 32, 64));
    font.setPixelSize(50);
    pen = QPen(Qt::white);
}

void GraphicsWidget::paint(QPainter & painter) const
{
    QRect rc  = painter.window();
    const auto & bids = g.Bids();
    const auto & asks = g.Asks();

    if (bids.empty() || asks.empty())
    {
        return;
    }

    QSize sz = size();
    int fontHeight = painter.fontMetrics().height();

    // calc xy ranges
    // use %age of values around mid price, binary find?

    double xrange = g.AmountMax().getAsDouble();
    double yrange = (g.PriceMax()-g.PriceMin()).getAsDouble();
    //double yOrg = ((priceMax + priceMin)/2).getAsDouble();
    double yOrg = ((bids.rbegin()->first + asks.begin()->first) / 2).getAsDouble();

    Decimal amount;
    bool first = true;
    float x = 0, y = 0;
    float width = sz.width() / 2.f;
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

    size_t lines = static_cast<size_t>(floor(height / 2 / fontHeight));
    size_t count = lines - 1;
    float gap = 60;//calc, dpi aware, use font width av
    y = height / 2 + fontHeight;
    // setFont
    for (auto it = bids.rbegin(); count != 0 && it != bids.rend(); ++it, --count)
    {
        painter.drawText(QRectF{ QPointF{width, y}, QPointF{width * 2, y + fontHeight} }, QString::number(it->first.getAsDouble(), 'f', 2));
        painter.drawText(QRectF{ QPointF{width + gap, y}, QPointF{width * 2, y + fontHeight} }, QString::number(it->second.getAsDouble(), 'f', 8));
        y += fontHeight;
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

    y = height / 2 - fontHeight;
    count = lines - 1;
    float pos = width;
    for (auto it = asks.begin(); count != 0 && it != asks.end(); ++it, --count)
    {
        y -= fontHeight;
        painter.drawText(QRectF{ QPointF{pos, y}, QPointF{pos * 2, y + fontHeight} }, QString::number(it->first.getAsDouble(), 'f', 2));
        painter.drawText(QRectF{ QPointF{pos+gap, y}, QPointF{pos * 2, y + fontHeight} }, QString::number(it->second.getAsDouble(), 'f', 8));
    }

    y = 0;
    count = static_cast<size_t>(floor(height / fontHeight));
    pos = width*1.3;
    gap = 70;
    for (auto it = g.Ticks().rbegin(); count != 0 && it != g.Ticks().rend(); ++it, --count)
    {
        painter.setPen(Qt::white);
        painter.drawText(QRectF{ QPointF{pos, y}, QPointF{pos * 2, y + fontHeight} }, QString::number(it->lastSize.getAsDouble(), 'f', 8));
        painter.setPen(it->side==Tick::Buy ? BidEdgeColour : AskEdgeColour);
        painter.drawText(QRectF{ QPointF{pos+gap, y}, QPointF{pos * 2, y + fontHeight} }, QString::number(it->price.getAsDouble(), 'f', 2));
        painter.setPen(Qt::white);
        painter.drawText(QRectF{ QPointF{pos+2*gap, y}, QPointF{pos * 2, y + fontHeight} }, it->time.toLocalTime().time().toString());
        y += fontHeight;
    }
}
