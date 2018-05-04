#ifndef TICK_H
#define TICK_H

#include "defs.h"
//#include <chrono>
#include <QDateTime>

class QJsonObject;

struct Tick
{
    static Tick fromJson(const QJsonObject &);

    enum Side { Buy, Sell };
    typedef unsigned long long TradeId;
    typedef unsigned long long SequenceNumber;
//    typedef std::chrono::system_clock Clock;
//    typedef std::chrono::time_point<Clock, std::chrono::microseconds> TimePoint;

    TradeId tradeId;
    SequenceNumber sequence;
    //TimePoint time;
    QDateTime time;
    Decimal price;
    Side side;
    Decimal lastSize;
    Decimal bestBid;
    Decimal bestAsk;

//     "time": "2017-09-02T17:05:49.250000Z",
//     "price": "4388.01000000",
//     "side": "buy", // Taker side
//     "last_size": "0.03000000",
//     "best_bid": "4388",
//     "best_ask": "4388.01"
};
#endif // TICK_H
