#ifndef CANDLECHART_H
#define CANDLECHART_H

#include "restprovider.h"

#include "plot.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPaintEvent>

#include <deque>

class CandleChart : public QOpenGLWidget
{
    static constexpr int MaxCandleWidth = 32;
    static constexpr int MinCandleWidth = 6;

    QColor background;
    Plot mutable candlePlot;
    std::deque<Candle> candles;
    time_t timeDelta;
    QPainterPath smaPath, emaPath;
    QPoint lastDrag;

public:
    CandleChart(QWidget *parent = 0);

    void SetCandles(std::deque<Candle> newCandles, Granularity granularity);

private:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter;
        painter.begin(this);
        painter.fillRect(event->rect(), background);
        painter.setRenderHint(QPainter::Antialiasing);
        //painter.setRenderHint(QPainter::HighQualityAntialiasing);
        Paint(painter);
        painter.end();
    }

    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *) override;

    void Paint(QPainter & painter) const;
};

#endif // CANDLECHART_H
