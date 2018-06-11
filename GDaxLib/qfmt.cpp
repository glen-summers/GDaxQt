#include "qfmt.h"
#include "formatter.h"
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QString>

std::ostream & operator << (std::ostream & stm, const QPoint & p)
{
    return Formatter::Format(stm, "({0},{1})", p.x(), p.y());
}

std::ostream & operator << (std::ostream & stm, const QPointF & p)
{
    return Formatter::Format(stm, "({0},{1})", p.x(), p.y());
}

std::ostream & operator << (std::ostream & stm, const QRect & p)
{
    return Formatter::Format(stm, "[{0} - {1}])", p.topLeft(), p.bottomRight());
}

std::ostream & operator << (std::ostream & stm, const QRectF & p)
{
    return Formatter::Format(stm, "[{0} - {1}])", p.topLeft(), p.bottomRight());
}

std::ostream & operator << (std::ostream & stm, const QString & s)
{
    return stm << s.toStdString();
}

