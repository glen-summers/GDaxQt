#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H

#include "plot.h"

#include "flogging.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPaintEvent>

#include <memory>

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
    void paintEvent(QPaintEvent *event) override
    {
        Flog::ScopeLog s(log, Flog::Level::Debug, "paintEvent");

        QPainter painter;
        painter.begin(this);
        painter.fillRect(event->rect(), background);
        painter.setRenderHint(QPainter::Antialiasing);
        Paint(painter);
        painter.end();
    }

private:
    void Paint(QPainter & painter) const;
};

#endif // GRAPHICSWIDGET_H
