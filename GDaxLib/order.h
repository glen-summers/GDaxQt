#ifndef ORDER_H
#define ORDER_H

#include "defs.h"
#include "decimalwrap.h"

#include <QString>
#include <QDateTime>

class QJsonValue;

enum class OrderType : unsigned { Limit, Market };
enum class OrderStatus : unsigned { Pending, Active, Rejected, Open, Done, Settled };
enum class SelfTradeProtection  : unsigned { None, DecrementAndCancel, CancelOldest, CancelNewest, CancelBoth };

struct Order
{
    // todo decouple QJson, use an opaque Deserialiser?
    static Order FromJson(const QJsonValue &value);

    QString id;
    Decimal price;
    Decimal size;
    QString productId;
    MakerSide side;
    SelfTradeProtection stp;
    OrderType type;
    //"time_in_force" : GTC(def), GTT, IOC, FOK
    bool PostOnly;
    QDateTime createdAt;
    //"fill_fees": "0.0000000000000000" prec?
    //"filled_size": "0.00000000", prec?
    //"executed_value": "0.0000000000000000"//prec?
    OrderStatus status;
    bool settled;
};

std::ostream & operator << (std::ostream & s, const Order &);

#endif // ORDER_H
