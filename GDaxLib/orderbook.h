#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "decimalwrap.h"

#include <QMutex>

#include <map>
#include <algorithm>

class OrderBook
{
    QMutex mutex;
    std::map<Decimal, Decimal> bids, asks;
    Decimal totBid;
    Decimal totAsk;

public:
    const QMutex & Mutex() const { return mutex; }

    const std::map<Decimal, Decimal> & Bids() const
    {
        return bids;
    }

    const std::map<Decimal, Decimal> & Asks() const
    {
        return asks;
    }

    void AddBid(const Decimal & price, const Decimal & amount)
    {
        bids[price] += amount;
        totBid += amount;
    }

    void UpdateBid(const Decimal & price, const Decimal & amount)
    {
        auto it = bids.find(price);
        if (it != bids.end())
        {
            totBid -= it->second;
        }
        if (amount == Decimal())
        {
            bids.erase(it);
        }
        else if (it != bids.end())
        {
            it->second = amount;
        }
        else
        {
            bids.insert({price, amount});
        }
        totBid += amount;
    }

    void AddAsk(const Decimal & price, const Decimal & amount)
    {
        asks[price] += amount;
        totAsk += amount;
    }

    void UpdateAsk(const Decimal & price, const Decimal & amount)
    {
        auto it = asks.find(price);
        if (it != asks.end())
        {
            totAsk -= it->second;
        }
        if (amount == Decimal())
        {
            asks.erase(it);
        }
        else if (it != asks.end())
        {
            it->second = amount;
        }
        else
        {
            asks.insert({price, amount});
        }
        totAsk += amount;
    }

    Decimal SeekAmount(const Decimal & priceLow, const Decimal & priceHigh) const
    {
        Decimal bidAmount, askAmount;
        for (auto bit = bids.rbegin(); bit!=bids.rend() && bit->first >= priceLow; ++bit)
        {
            bidAmount += bit->second;
        }
        for (auto ait = asks.begin(); ait!=asks.end() && ait->first <= priceHigh; ++ait)
        {
            askAmount += ait->second;
        }
        return std::max(bidAmount, askAmount);
    }

    Decimal MidPrice() const
    {
        auto bidIt = bids.rbegin();
        auto askIt = asks.begin();

        return bidIt != bids.rend() && askIt != asks.end()
            ? (bidIt->first + askIt->first)/2
            : Decimal{};
    }

    void Clear()
    {
        bids.clear();
        asks.clear();
    }
};


#endif // ORDERBOOK_H
