#ifndef DEFS_H
#define DEFS_H

#include <stdexcept>

// breakup and out of line...

// move
typedef unsigned long long TradeId;
typedef unsigned long long SequenceNumber;

enum class MakerSide : unsigned { None, Buy, Sell }; // down\up
enum class TakerSide : unsigned { None, Buy, Sell }; // up\down

inline const char * MakerSideToString(MakerSide side)
{
    switch (side)
    {
        case MakerSide::Buy:
            return "buy";
        case MakerSide::Sell:
            return "sell";
        default:
            throw std::runtime_error("Invalid side");
    }
}

template<typename T> T Unexpected(const std::string & value)
{
    throw std::runtime_error("Unexpected value: '" + value + "'");
}

inline MakerSide MakerSideFromString(const std::string & value)
{
    return value.empty()
        ? MakerSide::None
        : value == "buy"
            ? MakerSide::Buy
            : value == "sell"
                ? MakerSide::Sell
                : Unexpected<MakerSide>(value);
}


inline TakerSide TakerSideFromString(const std::string & value)
{
    return value.empty()
        ? TakerSide::None
        : value == "buy"
            ? TakerSide::Buy
            : value == "sell"
                ? TakerSide::Sell
                : Unexpected<TakerSide>(value);
}

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
