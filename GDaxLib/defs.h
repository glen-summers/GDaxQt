#ifndef DEFS_H
#define DEFS_H

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
