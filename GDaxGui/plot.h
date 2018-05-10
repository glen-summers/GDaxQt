#ifndef PLOT_H
#define PLOT_H

#include <QRectF>

QT_FORWARD_DECLARE_CLASS(QPainter)

class Plot
{
    double const edge;
    double const fontHeight;
    QRectF inner;
    QRectF view;

public:
    Plot(double edge);

    void setRect(const QRect & rect)
    {
        inner = rect;
        inner.adjust(edge, edge, -edge, -fontHeight-edge);
    }

    void setView(const QRectF & value);
    void startInner(QPainter & painter) const;
    void endInner(QPainter & painter) const;
    void drawTimeAxis(QPainter & painter) const;
    void drawCandle(QPainter & painter, double start, double end, double min, double max, double open, double close) const;

private:
    static std::pair<double, int> GetScaleAndDivisions(double range);
    static std::pair<double, int> GetTimeScale(double min, double max, int maxGaps);
};

#endif // PLOT_H
