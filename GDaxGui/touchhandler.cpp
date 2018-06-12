#include "touchhandler.h"

#include "flogging.h"
#include "qfmt.h"

#include <QPoint>
#include <QPointF>
#include <QRectF>
#include <QLineF>
#include <QMouseEvent>
#include <QEvent>
#include <QApplication>

#include <cmath>

namespace
{
    Flog::Log flog = Flog::LogManager::GetLog<TouchHandler>();
}

TouchHandler::TouchHandler(Touchee &touchee)
    : touchee(touchee)
    , isMouseTracking()
    , isTouchTracking()
    , tp0Id()
    , tp1Id()
    , lastDrag()
    , originalRect()
    , originalView()
{}

bool TouchHandler::Event(QEvent *event)
{
    /* QT bug? Touch fingers 1 > 2 >1 stops sending messages to mouse handler
        a fix cld be use use AA_SynthesizeTouchForUnhandledMouseEvents to handle all mouse\touch in touch handler
        but then QT it sends the pointer values using the wrong dpi values when dpi scaling is in effect
    */

    bool handled = false;
    switch (event->type())
    {
        case QEvent::MouseButtonPress:
        {
            auto mouseEvent = static_cast<QMouseEvent*>(event);
            lastDrag = mouseEvent->pos();
            qApp->setOverrideCursor(Qt::ClosedHandCursor);
            isMouseTracking = true;
            handled = true;
            break;
        }

        case QEvent::MouseButtonRelease:
        {
            qApp->restoreOverrideCursor();
            lastDrag = {};
            isMouseTracking = false;
            handled = true;
            break;
        }

        case QEvent::MouseMove:
        {
            if (!isTouchTracking && isMouseTracking && !lastDrag.isNull())
            {
                auto mouseEvent = static_cast<QMouseEvent*>(event);
                auto pos = mouseEvent->pos();
                flog.Debug("Mouse move {0}", pos);
                auto delta = pos - lastDrag;
                lastDrag = pos;
                touchee.Pan(delta.x(), delta.y());
                handled = true;
            }
            else
            {
                lastDrag = {};
            }
            break;
        }

        case QEvent::Wheel:
        {
            auto wheelEvent = static_cast<QWheelEvent *>(event);
            QPointF pos = wheelEvent->posF();
            double delta = wheelEvent->angleDelta().y()/1200.; // 120|15degs per click = +-0.1 per wheel tick
            double scale = delta>=0 ? 1+delta : 1 / (1-delta);
            touchee.ContextScale(pos, scale);
            handled = true;
            break;
        }

        case QEvent::TouchBegin:
        {
            flog.Debug("TouchBegin");
            handled = true;
            lastDrag = {};
            break;
        }

        case QEvent::TouchCancel:
        {
            flog.Debug("TouchCancel");
            isTouchTracking  = false;
            handled = true; // set to ignore?
            break;
        }

        case QEvent::TouchEnd:
        {
            flog.Debug("TouchEnd");
            isTouchTracking  = false;
            handled = true;
            break;
        }

        case QEvent::TouchUpdate:
        {
            auto touchEvent = static_cast<QTouchEvent *>(event);
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
                    flog.Debug("new touchPoint ids");
                }

                auto rc = QRectF(tp0.pos(), tp1.pos()).normalized();
                if (!isTouchTracking)
                {
                    tp0Id = tp0.id();
                    tp1Id = tp1.id();

                    originalRect = rc;
                    originalView = touchee.View();
                    flog.Debug("start touchTracking rect:{0} view:{1}, id0:{2}, id1:{3}", rc, touchee.View(), tp0Id, tp1Id);
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

                touchee.Scale(rc.center(), scaleX, scaleY);

                flog.Debug("touchTrack rect:{0} view:{1}, sx:{2}, sy:{3}, id0:{4}, id1:{5}", rc, touchee.View(), scaleX, scaleY,
                         tps[0].id(), tps[1].id());
            }
            else
            {
                flog.Debug("Update Tracking:{0}, Pts:{1}", isTouchTracking, tps.size());
            }

            if (isTouchTracking && tps.size() != 2)
            {
                isTouchTracking  = false;
                lastDrag = {};
                flog.Debug("touchTracking  = false;");
            }
            handled = true;
            break;
        }

        default:;
    }

    return handled;
}
