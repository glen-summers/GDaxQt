#ifndef CANDLE_H
#define CANDLE_H

#include "decimalwrap.h"

enum class Granularity : unsigned int
{
    Minutes=60,
    FiveMinutes=300,
    FifteenMinutes=900,
    Hours=3600,
    SixHours=21600,
    Days=86400
};

struct Candle
{
    time_t startTime;
    Decimal lowestPrice;
    Decimal highestPrice;
    Decimal openingPrice;
    Decimal closingPrice;
    Decimal volume;
};

struct CandleLess
{
    bool operator() (const Candle & lhs, const time_t & time) const
    {
        return lhs.startTime < time;
    }

    bool operator() (const time_t & time, const Candle & lhs) const
    {
        return time < lhs.startTime;
    }
};


#endif // CANDLE_H
