#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include <QPainter>

class OverlayWidget : public QWidget
{
public:
   OverlayWidget(QWidget * parent = nullptr) : QWidget(parent)
   {
      setAttribute(Qt::WA_NoSystemBackground);
      newParent();
   }

protected:
    bool eventFilter(QObject * obj, QEvent * ev) override
    {
        if (obj == parent())
        {
            if (ev->type() == QEvent::Resize)
            {
                resize(static_cast<QResizeEvent*>(ev)->size());
            }
            else if (ev->type() == QEvent::ChildAdded)
            {
                raise();
            }
        }
         return QWidget::eventFilter(obj, ev);
    }

    bool event(QEvent* ev) override
    {
      if (ev->type() == QEvent::ParentAboutToChange)
      {
          if (parent())
          {
              parent()->removeEventFilter(this);
          }
      }
      else if (ev->type() == QEvent::ParentChange)
      {
          newParent();
          raise();
      }
      return QWidget::event(ev);
   }

private:
    void newParent()
    {
        if (parent())
        {
            parent()->installEventFilter(this);
        }
    }
};

#endif // OVERLAYWIDGET_H
