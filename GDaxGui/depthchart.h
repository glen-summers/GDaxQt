#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H

#include "plot.h"

#include "flogging.h"

#include <QOpenGLWidget>

class GDaxProvider;

class DepthChart : public QOpenGLWidget
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<DepthChart>();

    Q_OBJECT

    QBrush background;
    Plot mutable depthPlot;
    const GDaxProvider * g;

public:
    DepthChart(QWidget *parent = nullptr);

    void SetProvider(const GDaxProvider * value)
    {
        g = value;
    }

private:
    void paintEvent(QPaintEvent *event) override;

private:
    void Paint(QPainter & painter) const;
};

#endif // GRAPHICSWIDGET_H
