#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H

#include "plot.h"
#include "gdl.h"

#include "flogging.h"

#include <QOpenGLWidget>

class DepthChart : public QOpenGLWidget
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<DepthChart>();
    static constexpr double orderbookFractionInitial = 0.003;
    static constexpr double orderbookFractionMin = 0.001;
    static constexpr double orderbookFractionMax = 0.1;

    Q_OBJECT

    QBrush background;
    Plot mutable depthPlot;
    const GDL::IStream * gdl;
    double orderbookFraction;

public:
    DepthChart(QWidget *parent = nullptr);

    void SetProvider(const GDL::IStream * value)
    {
        gdl = value;
    }

private:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void Paint(QPainter & painter) const;
};

#endif // GRAPHICSWIDGET_H
