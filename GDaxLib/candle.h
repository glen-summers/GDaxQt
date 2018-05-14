#ifndef CANDLE_H
#define CANDLE_H

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
