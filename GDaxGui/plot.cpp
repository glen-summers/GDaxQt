#include "plot.h"

#include <QApplication>
#include <QPainter>

#include <iomanip>
#include <sstream>
#include <algorithm>

Plot::Plot(double edge, bool xAxisLabels, bool yAxisLabels)
    : font(QApplication::font())
    , metrics(font)
    , edge(edge)
    , xAxisLabels(xAxisLabels)
    , yAxisLabels(yAxisLabels)
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
    painter.setPen(qRgb(180,180,180)); // window\custom styles? move

    // pixels = xScale * time;
    auto width = inner.width();
    double xScale = inner.width()/view.width();
    std::pair<double, int> pair = GetTimeScale(view.left(), view.right(), 10);
    double step = pair.first;
    int subDiv = pair.second;

    double offset = floor(xScale * view.left());
    double halfBox = step * xScale/2;
    int fh = metrics.height();
    double spacer = fh/4;
    double tickLen = fh/2;
    bool minor = true;

    const int minorThreshold = 5;
    if (xScale * step / subDiv < minorThreshold)
    {
        minor = false;
    }

    for (double x = step * floor(view.left() / step); x <= view.right(); x += step)
    {
        double j = xScale * x - offset;
        if (j >= 0 && j <= width)
        {
            j += inner.left();
            painter.drawLine(j, inner.bottom(), j, inner.bottom()-tickLen);

            if (xAxisLabels)
            {
                if (j >= 0 && j <= width)
                {

                    QRectF rc(QPointF{ j-halfBox, inner.bottom() + spacer}, QPointF{j+halfBox, outer.bottom()});
                    time_t tt = static_cast<time_t>(x);
                    tm tmLabel{};
                    gmtime_s(&tmLabel, &tt);
                    std::ostringstream stm;
                    if (tmLabel.tm_hour==0)
                    {
                        stm << std::put_time(&tmLabel, "%b %d");
                    }
                    else
                    {
                        stm << std::put_time(&tmLabel, "%H:%M");
                    }
                    painter.drawText(rc, Qt::AlignHCenter, QString(stm.str().c_str()));
                }
            }
        }

        if (minor)
        {
            double w = x;
            for (int i = 1; i < subDiv; ++i)
            {
                w += step / subDiv;
                double k = xScale * w - offset;
                if (k >= 0 && k <= width)
                {
                    k += inner.left();
                    painter.drawLine(k, inner.bottom(), k, inner.bottom() - tickLen / 2);
                }
            }
        }
    }
}

void Plot::drawYAxis(QPainter & painter, double position, bool drawLabels) const
{
    painter.setPen(qRgb(180,180,180)); // window\custom styles? move

    bool minor = true;

    std::pair<double, int> pair = GetScaleAndDivisions(view.height() / 10);
    double step = pair.first;
    int subDiv = pair.second;
    int decs = std::max(0, static_cast<int>(1 - log10(step)));

    double xPos = inner.left() + position * inner.width();
    int tickLen = metrics.averageCharWidth();
    int fh = metrics.height();
    int dir = position > 0.5 ? -1 : 1;

    double scale = inner.height() / view.height();
    double offset = floor(scale * view.top());

    const int minorThreshold = 5;
    if (scale*step / subDiv < minorThreshold)
    {
        minor = false;
    }

    auto align = Qt::TextSingleLine | Qt::AlignVCenter;

    for (double v = step * floor(view.top() / step); v <= view.bottom(); v += step)
    {
        double j = scale*v - offset;
        if (j >= 0 && j <= inner.height())
        {
            j = inner.bottom() - j;
            painter.drawLine(xPos, j, xPos + dir*tickLen, j);

            if (drawLabels)
            {
                auto qs = QString::number(v, 'f', decs);
                if (dir < 0)
                {
                    QRectF rc(QPointF{xPos + tickLen, j-fh}, QPointF{outer.right(), j+fh});
                    painter.drawText(rc, align | Qt::AlignLeft, qs);
                }
                else
                {
                    QRectF rc(0, j-fh, xPos - tickLen, fh);
                    painter.drawText(rc, align | Qt::AlignRight, qs);
                }
            }
        }
        if (minor)
        {
            double w = v;
            for (int i = 1; i< subDiv; ++i)
            {
                w += step / subDiv;
                double k = scale*w - offset;
                if (k >= 0 && k <= inner.height())
                {
                    k = inner.bottom() - k;
                    painter.drawLine(xPos, k, xPos + dir*tickLen/2, k);
                }
            }
        }
    }
    /*if (Min<0 && Max>0 && ShowZero)
    {
    MoveTo(dc,xPos,yPos-Height*Min/(Max-Min));
    LineTo(dc,xPos+Width,yPos-Height*Min/(Max-Min));
    }*/

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

double Plot::calcYAxisLabelWidth(double min, double max, double scale) const
{
    double step = GetScale((max-min)/scale/10, 1); // 10 steps, use param\constant
    int decs = std::max(0, static_cast<int>(1 - log10(step))); // floor?
    double value = step * floor(max / scale / step); // max od abs(max), abs(min)?

    QFontMetrics metrics(font); // cache
    return metrics.boundingRect(QString::number(value, 'g', decs)).width();
}

double Plot::GetScale(double range, double scale)
{
    static constexpr int lookup1[] = { 1,1,2,5,5,5,10,10,10,10,10 };
    double powx = pow(10.0, floor(log10(range / scale)))*scale;
    int ix = 1 + static_cast<int>(floor(range / powx - 1e-10));
    return powx == 0 ? range : lookup1[ix] * powx;
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
