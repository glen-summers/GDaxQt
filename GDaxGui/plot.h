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

    void SetRect(const QRect & rect)
    {
        outer = inner = rect;
        inner.adjust(edge, edge, -edge, -edge);
        if (xAxisLabels)
        {
            inner.adjust(0, 0, 0, -metrics.height());
        }
        if (yAxisLabels)
        {
            double width = CalcYAxisLabelWidth(view.top(), view.bottom(), 1);
            inner.adjust(0, 0, -width, 0);
        }
    }

    const QRectF Inner() const { return inner; }

    void SetView(const QRectF & value);
    QRectF View() const { return view; }
    QPointF MapToView(const QPointF & p) const;

    QPointF ScaleToScreen(const QPointF & p) const
    {
        return p * inner.width() / view.width();
    }

    void ZoomY(const QPointF & at, double scale);
    void ZoomX(const QPointF & at, double scale);
    void Pan(double dx, double dy);

    void StartInner(QPainter & painter) const;
    void EndInner(QPainter & painter) const;
    void DrawTimeAxis(QPainter & painter) const;
    void DrawYAxis(QPainter & painter, double position, bool drawLabels) const;
    void DrawCandle(QPainter & painter, double start, double end, double min, double max, double open, double close) const;

    double CalcYAxisLabelWidth(double min, double max, double scale) const;

private:
    static double GetScale(double range, double scale);
    static std::pair<double, int> GetScaleAndDivisions(double range);
    static std::pair<double, int> GetTimeScale(double min, double max, int maxGaps);
};

#endif // PLOT_H
