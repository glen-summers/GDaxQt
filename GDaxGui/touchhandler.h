#ifndef TOUCHHANDLER_H
#define TOUCHHANDLER_H

#include <QPoint>
#include <QRectF>
class QPointF;
class QEvent;

struct Touchee
{
    virtual const QRectF & View() const  = 0;
    virtual void SetView(const QRectF & view) const  = 0;
    virtual void Pan(double x, double y) = 0;
    virtual void Scale(const QPointF & p, double xScale, double yScale) = 0;
    virtual void ContextScale(const QPointF & p, double scale) = 0;
};

class TouchHandler
{
    Touchee & touchee;
    bool isMouseTracking;
    bool isTouchTracking;
    int tp0Id;
    int tp1Id;
    QPoint lastDrag;
    QRectF originalRect;
    QRectF originalView;

public:
    TouchHandler(Touchee & touchee);

    bool Event(QEvent * event);
};

#endif // TOUCHHANDLER_H

