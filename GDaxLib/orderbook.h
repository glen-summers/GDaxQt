#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "decimalwrap.h"

#include <QMutex>

#include <map>

class OrderBook
{
    QMutex mutex;
    std::map<Decimal, Decimal> bids, asks;
    Decimal priceMin;
    Decimal priceMax;
    Decimal amountMax;

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

    const Decimal & PriceMin() const
    {
        return priceMin;
    }

    const Decimal & PriceMax() const
    {
        return priceMax;
    }

    const Decimal & AmountMax() const
    {
        return amountMax;
    }

    void AddBid(const Decimal & price, const Decimal & amount)
    {
        bids[price] += amount;
    }

    void UpdateBid(const Decimal & price, const Decimal & amount)
    {
        if (amount == Decimal())
        {
            bids.erase(price);
        }
        else
        {
            bids[price] = amount;
        }
    }

    void AddAsk(const Decimal & price, const Decimal & amount)
    {
        asks[price] += amount;
    }

    void UpdateAsk(const Decimal & price, const Decimal & amount)
    {
        if (amount == Decimal())
        {
            asks.erase(price);
        }
        else
        {
            asks[price] = amount;
        }
    }

    void SeekRange(const Decimal & target)
    {
        Decimal tot;
        auto bidIt = bids.rbegin();
        auto askIt = asks.begin();
        for(; tot<target;++bidIt,++askIt)
        {
            if (bidIt!=bids.rend())
            {
                tot += bidIt->second;
            }
            if (askIt!=asks.end())
            {
                tot += askIt->second;
            }
        }
        // check for end
        priceMin = bidIt->first;
        priceMax = askIt->first;
        amountMax = tot;
    }

    void Clear()
    {
        bids.clear();
        asks.clear();
        priceMin = Decimal();
        priceMax = Decimal();
        amountMax = Decimal();
    }
};


#endif // ORDERBOOK_H
