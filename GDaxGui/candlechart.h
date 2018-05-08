#ifndef CANDLECHART_H
#define CANDLECHART_H

#include "restprovider.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPaintEvent>

class CandleChart : public QOpenGLWidget
{
    QBrush background;
    std::vector<Candle> candles;

public:
    CandleChart(QWidget *parent = 0);

    void setCandles(std::vector<Candle> candles);

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

    void paint(QPainter & painter) const;
};

#endif // CANDLECHART_H
