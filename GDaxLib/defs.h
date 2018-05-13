#ifndef DEFS_H
#define DEFS_H

#define DEC_NAMESPACE DecNs
#include "decimal.h"

// 64 bit impl == 18 digits
typedef DecNs::decimal<8> Decimal;

// move
typedef unsigned long long TradeId;
typedef unsigned long long SequenceNumber;

enum class MakerSide { None, Buy, Sell }; // down\up
enum class TakerSide { None, Buy, Sell }; // up\down

inline MakerSide ToMaker(TakerSide side)
{
    switch (side)
    {
        case TakerSide::Buy:
            return MakerSide::Sell;

        case TakerSide::Sell:
            return  MakerSide::Buy;
    }
    return MakerSide::None;
}

#endif // DEFS_H
