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

#endif // CANDLE_H
