#include "candlechart.h"

#include <QApplication>

CandleChart::CandleChart(QWidget *parent)
    : QOpenGLWidget(parent)
    , background(Qt::black)
{
}

void CandleChart::setCandles(std::vector<Candle> candles)
{
    this->candles = std::move(candles);
}

// to utils
std::pair<double, int> GetScaleAndDivisions(double range)
{
    static constexpr int lookup1[] = { 1,1,2,5,5,5,10,10,10,10,10 };
    static constexpr int lookup2[] = { 5,5,4,5,5,5,5,5,5,5,5 };
    double powx = pow(10.0, floor(log10(range)));
    int ix = powx == 0 ? 0 : 1 + static_cast<int>(floor(range / powx - 1e-10));
    return { lookup1[ix] * powx, lookup2[ix] };
}

// to utils
std::pair<double, int> GetTimeScale(double min, double max, int maxGaps)
{
    double r = static_cast<double>(max - min) / maxGaps;

    if (r < 1)
    {
        // < 1sec
        return GetScaleAndDivisions(r);
    }

    if (r < 60)
    {
        // 1sec - 1min
        // 1,2,5,10,15,20,30,60 sec
        int n = static_cast<int>(floor(r));
        if (n < 1) return { 1,5 };
        if (n < 2) return { 2,4 };
        if (n < 5) return { 5, 5 };
        if (n < 10) return { 10,5 };
        if (n < 15) return { 15,3 };
        if (n < 20) return { 20,4 };
        if (n < 30) return { 30,3 };
        return { 60,4 };
    }

    if (r < 60.0 * 60)
    {
        // 1min - 1hr
        // 1,2,5,10,15,20,30 min
        const int f = 60;
        int n = static_cast<int>(floor(r / f));
        if (n < 1) return { 1 * f,4 };
        if (n < 2) return { 2 * f,4 };
        if (n < 4) return { 5 * f,5 };
        if (n < 10) return { 10 * f,5 };
        if (n < 15) return { 15 * f,3 };
        if (n < 15) return { 15 * f,3 };
        if (n < 20) return { 20 * f,4 };
        if (n < 30) return { 30 * f,3 };
        return { 60 * f,4 };
    }

    if (r < 24.0 * 60 * 60)
    {
        // 1hr - 1d
        // 1,2,3,4,6,12,24
        const int f = 60 * 60;
        int n = static_cast<int>(floor(r / f));
        if (n < 1) return { 1 * f,4 };
        if (n < 2) return { 2 * f,4 };
        if (n < 3) return { 3 * f,3 };
        if (n < 4) return { 4 * f,4 };
        if (n < 6) return { 6 * f,3 };
        if (n < 12) return { 12 * f,3 };
        return { 24 * f,4 };
    }

    // > 1d
    const int f = 24 * 60 * 60;
    double days = static_cast<double>(max - min) / (f* maxGaps);
    std::pair<double, int> p = GetScaleAndDivisions(days);
    p.first *= f;
    return p;
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

    QFont font = QApplication::font();
    QFontMetrics fm(font);
    int fontHeight = fm.height();

    QRectF inner = {{}, size()};
    int edge = 10;
    inner.adjust(edge, edge, -edge, -fontHeight-edge);
    painter.setPen(QPen(QColor(qRgb(80,80,80))));
    painter.drawRect(inner);

    //time_t startTime = candles.rbegin()->startTime;
    //time_t endTime = candles.begin()->startTime + 60*60;

    time_t startTime = time(nullptr); // or manual scroll value

    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
    for (const auto & c : candles)
    {
        max = std::max(max, c.highestPrice.getAsDouble());
        min = std::min(min, c.lowestPrice.getAsDouble());
    }

    // mode = 1hr... others
    time_t timeDelta = 60*60; //1d init, max = endTime - startTime;
    float xScale = 500.f/24/60/60; // init 500px per day, to cfg
    double yRange = (max-min);
    double wedgie = 0.3*timeDelta*xScale;

    float width = inner.width();
    float height = inner.height();

    // to cfg
    QPen penUp(QColor(qRgb(0,255,0)), 2);
    QBrush brushUp(QColor(qRgb(0,0,0)));
    QPen penDown(QColor(qRgb(255,0,0)), 2);
    QBrush brushDown(QColor(qRgb(255,0,0)));

    painter.setClipRect(inner);
    for (const auto & c : candles) // goes back in time...
    {
        float x = inner.right() - (startTime - c.startTime) * xScale; // from right!
        auto y0 = inner.top() + static_cast<float>(((max-c.lowestPrice.getAsDouble()) / yRange) * height);
        auto y1 = inner.top() + static_cast<float>(((max-c.highestPrice.getAsDouble()) / yRange) * height);
        auto yy0 = inner.top() + static_cast<float>(((max-c.openingPrice.getAsDouble()) / yRange) * height);
        auto yy1 = inner.top() + static_cast<float>(((max-c.closingPrice.getAsDouble()) / yRange) * height);

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
    painter.setClipping(false);

    auto maxTime = startTime;
    auto minTime = startTime - inner.width() / xScale;
    std::pair<double, int> pair = GetTimeScale(minTime, maxTime, 10);
    double step = pair.first;
    //int subDiv = pair.second;

    double offset = floor(xScale * minTime);
    float box2 = static_cast<float>(step * xScale)/2;

    painter.setPen(qRgb(180,180,180)); // window\custom styles?
    for (double x = step * floor(minTime / step); x <= maxTime; x += step)
    {
        float j = static_cast<float>(xScale * x - offset);
        if (j >= 0 && j <= width)
        {
            j += inner.left();

            QRectF rc = {QPointF{ j-box2, inner.bottom() + fontHeight / 4}, QPointF{j+box2, inner.bottom() + fontHeight *5/ 4}};
            time_t tt = static_cast<time_t>(x);
            tm tmLabel{};
            gmtime_s(&tmLabel, &tt); // should be localtime??
            std::ostringstream stm;
            stm << std::put_time(&tmLabel, "%b %d %H:%M"); // put date out on midnights

            //font->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            //font->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
            painter.drawText(rc, Qt::AlignHCenter, QString(stm.str().c_str()));
        }
    }
}
