#ifndef QFMT_H
#define QFMT_H

#include <iosfwd>

class QPoint;
class QPointF;
class QRect;
class QRectF;
class QString;
class QDateTime;

std::ostream & operator << (std::ostream & stm, const QPoint & p);
std::ostream & operator << (std::ostream & stm, const QPointF & p);
std::ostream & operator << (std::ostream & stm, const QRect & p);
std::ostream & operator << (std::ostream & stm, const QRectF & p);
std::ostream & operator << (std::ostream & stm, const QString & s);
std::ostream & operator << (std::ostream & stm, const QDateTime & s);

#endif // QFMT_H
