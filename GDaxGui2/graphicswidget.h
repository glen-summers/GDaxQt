#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H

#include "gdaxlib.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPaintEvent>

#include <memory>

class GraphicsWidget : public QOpenGLWidget
{
    Q_OBJECT

    const GDaxLib & g;

    QBrush background;
    QFont font;
    QPen pen;

public:
    GraphicsWidget(const GDaxLib & g, QWidget *parent = 0);

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
