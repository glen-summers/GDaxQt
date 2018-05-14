#include "plot.h"

#include <QApplication>
#include <QPainter>
#include <QDateTime>

#include <iomanip>
#include <sstream>
#include <algorithm>

#include <math.h>

Plot::Plot(double edge, bool xAxisLabels, bool yAxisLabels)
    : font(QApplication::font())
    , metrics(font)
    , edge(edge)
    , xAxisLabels(xAxisLabels)
    , yAxisLabels(yAxisLabels)
{
}

void Plot::SetView(const QRectF & value)
{
    view = value;
}

void Plot::StartInner(QPainter &painter) const
{
    painter.setPen(QPen(QColor(qRgb(80,80,80))));
    painter.drawRect(inner);
    painter.setClipRect(inner);
}

void Plot::EndInner(QPainter & painter) const
{
    painter.setClipping(false);
}

void Plot::DrawTimeAxis(QPainter &painter) const
{
    painter.setPen(qRgb(180,180,180)); // window\custom styles? move

    // pixels = xScale * time;
    auto width = inner.width();
    double xScale = inner.width()/view.width();
    std::pair<double, int> pair = GetTimeScale(view.left(), view.right(), 10);
    double step = pair.first;
    int subDiv = pair.second;

    auto utcOffset = QDateTime::currentDateTimeUtc().toLocalTime().offsetFromUtc();
    auto minLocal = view.left() + utcOffset;
    auto maxLocal = view.right() + utcOffset;

    double offset = floor(xScale * minLocal);
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

    for (double localTime_t = step * floor(minLocal / step); localTime_t <= maxLocal; localTime_t += step)
    {
        double j = xScale * localTime_t - offset;
        if (j >= 0 && j <= width)
        {
            j += inner.left();
            painter.drawLine(j, inner.bottom(), j, inner.bottom()-tickLen);

            if (xAxisLabels)
            {
                if (j >= 0 && j <= width)
                {
                    QRectF rc(QPointF{ j-halfBox, inner.bottom() + spacer}, QPointF{j+halfBox, outer.bottom()});

                    QDateTime qdt;
                    qdt.setTime_t(localTime_t-utcOffset);
                    QString time = (qdt.time().hour()==0)
                            ? qdt.toString("dd MMM")
                            : qdt.toString("hh:mm");

                    painter.drawText(rc, Qt::AlignHCenter, time);
                }
            }
        }

        if (minor)
        {
            double w = localTime_t;
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

void Plot::DrawYAxis(QPainter & painter, double position, bool drawLabels) const
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

void Plot::DrawCandle(QPainter & painter, double start, double end, double min, double max,
                      double open, double close, QBrush * fillBrush) const
{
    painter.save();

    painter.translate(inner.left(), inner.top());
    painter.scale(inner.width()/view.width(), -inner.height()/view.height());
    painter.translate(-view.left(), -view.bottom());

    auto p0 = QPointF{start, min};
    auto p1 = QPointF{start, max};
    painter.drawLine(p0, p1);

    double pixels = painter.transform().mapRect(QRectF{0,0,end-start,0}).width()/2 - 2;
    if (pixels > 1)
    {
        double wedgie = pixels*view.width()/inner.width(); //no reverse transform?

        if (fillBrush) // fix for scaled pen\brush pxelation. just draw fill
        {
            painter.fillRect(QRectF(QPointF(start - wedgie, open), QPointF(start + wedgie, close)), *fillBrush);
        }
        else
        {
            painter.drawRect(QRectF(QPointF(start - wedgie, open), QPointF(start + wedgie, close)));
        }
    }
    painter.restore();
}

double Plot::CalcYAxisLabelWidth(double min, double max, double scale) const
{
    double step = GetScale((max-min)/scale/10, 1); // 10 steps, use param\constant
    int decs = std::max(0, static_cast<int>(1 - log10(step))); // floor?
    double value = step * floor(max / scale / step); // max od abs(max), abs(min)?

    return metrics.boundingRect(QString::number(value, 'g', decs)).width();
}

QPointF Plot::MapToView(const QPointF & p) const
{
    return
    {
        // transform?
        view.x() + (p.x()-inner.x())/inner.width() * view.width(),
        view.y() + (inner.bottom()-p.y())/inner.height() * view.height(),
    };
}

void Plot::ZoomY(const QPointF &at, double scale)
{
    double mappedY = MapToView(at).y();
    view.adjust(0,(mappedY - view.top())*scale, 0, (mappedY - view.bottom())*scale);
}

void Plot::ZoomX(const QPointF &at, double scale)
{
    double mappedX = MapToView(at).x();
    view.adjust((mappedX - view.left())*scale, 0, (mappedX - view.right())*scale, 0);
}

void Plot::Pan(double dx, double dy)
{
    view.translate(-dx/inner.width() * view.width(), dy/inner.height() * view.height());
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
