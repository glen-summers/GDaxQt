#include "plot.h"

#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>

#include <iomanip>
#include <sstream>
#include <algorithm>

namespace Detail
{
    int getFontHeight()
    {
        QFont font = QApplication::font();
        QFontMetrics fm(font);
        return fm.height();
    }
}

Plot::Plot(double edge, bool xAxisLabels)
    : edge(edge)
    , xAxisLabels(xAxisLabels)
    , fontHeight(Detail::getFontHeight())
{
}

void Plot::setView(const QRectF & value)
{
    view = value;
}

void Plot::startInner(QPainter &painter) const
{
    painter.setPen(QPen(QColor(qRgb(80,80,80))));
    painter.drawRect(inner);
    painter.setClipRect(inner);
}

void Plot::endInner(QPainter & painter) const
{
    painter.setClipping(false);
}

void Plot::drawTimeAxis(QPainter &painter) const
{
    // pixels = xScale * time;
    auto width = inner.width();
    double xScale = inner.width()/view.width();
    std::pair<double, int> pair = GetTimeScale(view.left(), view.right(), 10);
    double step = pair.first;
    //int subDiv = pair.second;

    double offset = floor(xScale * view.left());
    double box2 = step * xScale/2;

    painter.setPen(qRgb(180,180,180)); // window\custom styles?
    for (double x = step * floor(view.left() / step); x <= view.right(); x += step)
    {
        // draw ticks

        if (xAxisLabels)
        {
            double j = xScale * x - offset;
            if (j >= 0 && j <= width)
            {
                j += inner.left();

                QRectF rc = {QPointF{ j-box2, inner.bottom() + fontHeight / 4}, QPointF{j+box2, inner.bottom() + fontHeight *5/ 4}};
                time_t tt = static_cast<time_t>(x);
                tm tmLabel{};
                gmtime_s(&tmLabel, &tt); // should be localtime??
                std::ostringstream stm;
                stm << std::put_time(&tmLabel, "%b %d %H:%M"); // put date out on midnights
                painter.drawText(rc, Qt::AlignHCenter, QString(stm.str().c_str()));
            }
        }
    }
}

void Plot::drawCandle(QPainter & painter, double start, double end, double min, double max, double open, double close) const
{
    // use a painter transform?
    double x0 = inner.left() + (start - view.left()) * inner.width() / view.width();
    double y0 = inner.bottom() - (min - view.top()) * inner.height() / view.height();
    double y1 = inner.bottom() - (max - view.top()) * inner.height() / view.height();
    painter.drawLine(QPointF(x0, y0), QPointF(x0, y1));

    auto yy0 = inner.bottom() - (open - view.top()) * inner.height() / view.height();
    auto yy1 = inner.bottom() - (close - view.top()) * inner.height() / view.height();

    double wedgie = std::min(1., (end-start) * inner.width() / view.width() /2 - 1);
    painter.drawRect(QRectF(QPointF(x0 - wedgie, yy0), QPointF(x0 + wedgie, yy1)));
}

std::pair<double, int> Plot::GetScaleAndDivisions(double range)
{
    static constexpr int lookup1[] = { 1,1,2,5,5,5,10,10,10,10,10 };
    static constexpr int lookup2[] = { 5,5,4,5,5,5,5,5,5,5,5 };
    double powx = pow(10.0, floor(log10(range)));
    int ix = powx == 0 ? 0 : 1 + static_cast<int>(floor(range / powx - 1e-10));
    return { lookup1[ix] * powx, lookup2[ix] };
}

std::pair<double, int> Plot::GetTimeScale(double min, double max, int maxGaps)
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
