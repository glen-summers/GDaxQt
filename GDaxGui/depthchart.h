#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H

#include "plot.h"

#include "flogging.h"

#include <QOpenGLWidget>

class GDaxLib;

class DepthChart : public QOpenGLWidget
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<DepthChart>();

    Q_OBJECT

    QBrush background;
    Plot mutable depthPlot;
    const GDaxLib * g;

public:
    DepthChart(QWidget *parent = nullptr);

    void SetGDaxLib(const GDaxLib * value)
    {
        g = value;
    }

private:
    void paintEvent(QPaintEvent *event) override;

private:
    void Paint(QPainter & painter) const;
};

#endif // GRAPHICSWIDGET_H
