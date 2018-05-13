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

inline TakerSide ToTaker(MakerSide side)
{
    switch (side)
    {
        case MakerSide::Buy:
            return TakerSide::Sell;

        case MakerSide::Sell:
            return  TakerSide::Buy;
    }
    return TakerSide::None;
}

#endif // DEFS_H
