#include "expandbutton.h"

ExpandButton::ExpandButton(QWidget *parent)
    : QToolButton(parent)
    , controlled()
{
    connect(this, &ExpandButton::toggled, this, &ExpandButton::Toggle);

    setStyleSheet("padding: 0px;");
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setArrowType(Qt::ArrowType::RightArrow);
    setCheckable(true);
    setChecked(false);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setIconSize({12,12});
}

void ExpandButton::setControlled(QWidget *widget)
{
    controlled = widget;
}

void ExpandButton::Toggle()
{
    setArrowType(isChecked() ? Qt::ArrowType::LeftArrow : Qt::ArrowType::RightArrow);
    if (controlled)
    {
        if (isChecked())
        {
            controlled->hide();
        }
        else
        {
            controlled->show();
        }
    }
}
