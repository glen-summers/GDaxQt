#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H

#include "plot.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPaintEvent>

#include <memory>

class GDaxLib;

class DepthChart : public QOpenGLWidget
{
    Q_OBJECT

    QBrush background;
    Plot mutable depthPlot;
    const GDaxLib * g;

public:
    DepthChart(QWidget *parent = 0);

    void setGDaxLib(const GDaxLib * value)
    {
        g = value;
    }

    void update()
    {
        repaint();
    }

private:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter;
        painter.begin(this);
        painter.fillRect(event->rect(), background);
        painter.setRenderHint(QPainter::Antialiasing);
        paint(painter);
        painter.end();
    }

private:
    void paint(QPainter & painter) const;
};

#endif // GRAPHICSWIDGET_H
