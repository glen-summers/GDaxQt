#ifndef TRADE_H
#define TRADE_H

#include "defs.h"
#include "decimalwrap.h"

#include <QDateTime>

class QJsonObject;

class Trade
{
public:
    typedef unsigned long long TradeId;

    static Trade FromJson(const QJsonObject & object);

    QDateTime time;
    TradeId tradeId;
    Decimal price;
    Decimal size;
    MakerSide side;
};

#endif // TRADE_H
