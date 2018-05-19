#ifndef CANDLEOVERLAY_H
#define CANDLEOVERLAY_H

#include "overlaywidget.h"

class CandleChart;

class CandleOverlay : public OverlayWidget
{
    const CandleChart & candleChart;
    time_t candleTime;

public:
    CandleOverlay(CandleChart &candleChart);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *) override;
};

#endif // CANDLEOVERLAY_H
