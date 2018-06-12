#ifndef TOUCHHANDLER_H
#define TOUCHHANDLER_H

#include "qfmt.h"

#include <QRectF>
#include <QMouseEvent>
#include <QApplication>

#include <flogging.h>

struct Touchee
{
    virtual const QRectF & View() const  = 0;
    virtual void SetView(const QRectF & view) const  = 0;
    virtual void Pan(double x, double y) = 0;
    virtual void Scale(const QPointF & p, double xScale, double yScale) = 0;
};

class TouchHandler
{
    inline static Flog::Log log = Flog::LogManager::GetLog<TouchHandler>();

    Touchee & touchee;
    bool isMouseTracking;
    bool isTouchTracking;
    int tp0Id;
    int tp1Id;
    QPoint lastDrag;
    QRectF originalRect;
    QRectF originalView;

public:
    TouchHandler(Touchee & touchee)
        : touchee(touchee)
        , isMouseTracking()
        , isTouchTracking()
        , tp0Id()
        , tp1Id()
        , lastDrag()
        , originalRect()
        , originalView()
        {}

    void MousePress(QMouseEvent *event)
    {
        lastDrag = event->pos();
        qApp->setOverrideCursor(Qt::ClosedHandCursor);
        isMouseTracking = true;
        event->accept();
    }

    void MouseMove(QMouseEvent *event)
    {
        if (!isTouchTracking)
        {
            auto delta = event->pos() - lastDrag;
            //log.Info("MouseMove {0}, [{1}]", event->pos(), delta);
            lastDrag = event->pos();
            touchee.Pan(delta.x(), delta.y());
            event->accept();
        }
    }

    void MouseRelease(QMouseEvent *event)
    {
        qApp->restoreOverrideCursor();
        event->accept();
        isMouseTracking = false;
    }

    // just handle mouse here as well
    bool Event(QEvent * event)
    {
        switch (event->type())
        {
            case QEvent::TouchBegin:
            {
                event->accept();
                return true; // same as accept?
            }

            case QEvent::TouchCancel:
            {
                log.Info("TouchCancel");
                // set to ignore?
                return true;
            }

            case QEvent::TouchEnd:
            {
                log.Info("TouchEnd");
                isTouchTracking  = false;
                event->accept();
                return true; // same as accept?
            }

            case QEvent::TouchUpdate:
            {
                QTouchEvent * touchEvent = (QTouchEvent *)event;
                auto tps = touchEvent->touchPoints();

                // AA_SynthesizeTouchForUnhandledMouseEvents
                // if (tps.size()==1 && !isMouseTracking)...

                if (tps.size() == 2)
                {
                    auto & tp0 = tps[0];
                    auto & tp1 = tps[1];

                    if (isTouchTracking && (tp0Id != tp0.id() || tp1Id != tp1.id()))
                    {
                        isTouchTracking  = false;
                        log.Info("new touchPoint ids");
                    }

                    auto rc = QRectF(tp0.pos(), tp1.pos()).normalized();
                    if (!isTouchTracking)
                    {
                        tp0Id = tp0.id();
                        tp1Id = tp1.id();

                        originalRect = rc;
                        originalView = touchee.View();
                        event->accept();
                        log.Info("start touchTracking rect:{0} view:{1}, id0:{2}, id1:{3}", rc, touchee.View(), tp0Id, tp1Id);
                        return isTouchTracking = true;
                    }

                    touchee.SetView(originalView);
                    auto delta = rc.center() - originalRect.center();
                    touchee.Pan(delta.x(), delta.y());

                    auto origLength = QLineF(originalRect.topLeft(), originalRect.bottomRight()).length(); //0? cache?
                    auto length = QLineF(rc.topLeft(), rc.bottomRight()).length();
                    auto scale = length/origLength - 1;
                    auto t2 = atan2(rc.height(), rc.width());
                    auto scaleX = 1 + scale * cos(t2);
                    auto scaleY = 1 + scale * sin(t2);

                    //auto minScale = MinCandleWidth * touchee.View().width() / timeDelta / touchee.Inner().width();
                    //auto maxScale = MaxCandleWidth * touchee.View().width() / timeDelta / touchee.Inner().width();
                    //if (scaleX < minScale) scaleX = minScale;
                    //if (scaleX > maxScale) scaleX = maxScale;
                    //touchee.ZoomX(rc.center(), scaleX);
                    //touchee.ZoomY(rc.center(), scaleY);
                    touchee.Scale(rc.center(), scaleX, scaleY);
                    //update();

                    log.Info("touchTrack rect:{0} view:{1}, sx:{2}, sy:{3}, id0:{4}, id1:{5}", rc, touchee.View(), scaleX, scaleY,
                             tps[0].id(), tps[1].id());
                }

                if (isTouchTracking && tps.size() != 2)
                {
                    isTouchTracking  = false;
                    log.Info("touchTracking  = false;");
                }
                event->accept();
                return true;
            }
        }
        return false;
    }
};

#endif // TOUCHHANDLER_H
