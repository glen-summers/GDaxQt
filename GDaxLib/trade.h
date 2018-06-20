#ifndef TRADE_H
#define TRADE_H

#include "defs.h"
#include "decimalwrap.h"

#include <QDateTime>

class QJsonValue;

struct Trade
{
    typedef unsigned long long TradeId;

    static Trade FromJson(const QJsonValue & object);

    QDateTime time;
    TradeId tradeId;
    Decimal price;
    Decimal size;
    MakerSide side;
};

#endif // TRADE_H
