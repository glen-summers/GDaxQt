#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "decimalwrap.h"

#include <QMutex>

#include <map>

class OrderBook
{
    QMutex mutex;
    std::map<Decimal, Decimal> bids, asks;
    Decimal totBid;
    Decimal totAsk;

    Decimal mutable priceMin; // move out
    Decimal mutable priceMax; // move out
    Decimal mutable amountMax;  // move out

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

    void SeekRange(const int permille) const
    {
        Decimal target = (totBid + totAsk) * permille/1000;
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
