#ifndef QFMT_H
#define QFMT_H

#include <iosfwd>

class QPoint;
class QPointF;
class QRect;
class QRectF;
class QString;

std::ostream & operator << (std::ostream & stm, const QPoint & p);
std::ostream & operator << (std::ostream & stm, const QPointF & p);
std::ostream & operator << (std::ostream & stm, const QRect & p);
std::ostream & operator << (std::ostream & stm, const QRectF & p);
std::ostream & operator << (std::ostream & stm, const QString & s);

#endif // QFMT_H
