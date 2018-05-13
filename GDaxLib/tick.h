#ifndef TICK_H
#define TICK_H

#include "defs.h"

#include <QDateTime>

class QJsonObject;

struct Tick
{
    static Tick FromJson(const QJsonObject &);

    TradeId tradeId;
    SequenceNumber sequence;
    QDateTime time;
    Decimal price;
    TakerSide side;
    Decimal lastSize;
    Decimal bestBid;
    Decimal bestAsk;
};
#endif // TICK_H
