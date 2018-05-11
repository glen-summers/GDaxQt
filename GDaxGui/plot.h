#ifndef PLOT_H
#define PLOT_H

#include <QRectF>
#include <QFont>
#include <QFontMetrics>

QT_FORWARD_DECLARE_CLASS(QPainter)

class Plot
{
    QFont const font;
    QFontMetrics const metrics;
    double const edge;
    bool const xAxisLabels;
    bool const yAxisLabels;
    QRectF outer;
    QRectF inner;
    QRectF view;

public:
    Plot(double edge, bool xAxisLabels, bool yAxisLabels);

    void setRect(const QRect & rect)
    {
        outer = inner = rect;
        inner.adjust(edge, edge, -edge, -edge);
        if (xAxisLabels)
        {
            inner.adjust(0, 0, 0, -metrics.height());
        }
        if (yAxisLabels)
        {
            double width = calcYAxisLabelWidth(view.top(), view.bottom(), 1);
            inner.adjust(0, 0, -width, 0);
        }
    }

    const QRectF getInner() const { return inner; }

    void setView(const QRectF & value);
    void startInner(QPainter & painter) const;
    void endInner(QPainter & painter) const;
    void drawTimeAxis(QPainter & painter) const;
    void drawYAxis(QPainter & painter, double position, bool drawLabels) const;
    void drawCandle(QPainter & painter, double start, double end, double min, double max, double open, double close) const;

    double calcYAxisLabelWidth(double min, double max, double scale) const;

private:
    static double GetScale(double range, double scale);
    static std::pair<double, int> GetScaleAndDivisions(double range);
    static std::pair<double, int> GetTimeScale(double min, double max, int maxGaps);
};

#endif // PLOT_H
