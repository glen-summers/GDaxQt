#ifndef DEFS_H
#define DEFS_H

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(push,3)
#endif

#define DEC_NAMESPACE DecNs
#include "decimal.h"
#undef DEC_NAMESPACE

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// 64 bit impl == 18 digits
typedef DecNs::decimal<8> Decimal;

// move
typedef unsigned long long TradeId;
typedef unsigned long long SequenceNumber;

enum class MakerSide { None, Buy, Sell }; // down\up
enum class TakerSide { None, Buy, Sell }; // up\down

inline MakerSide TakerToMaker(TakerSide side)
{
    switch (side)
    {
        case TakerSide::Buy:
            return MakerSide::Sell;

        case TakerSide::Sell:
            return  MakerSide::Buy;

        case TakerSide::None:
            break;
    }
    return MakerSide::None;
}

#endif // DEFS_H
