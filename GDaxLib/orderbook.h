#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "decimalwrap.h"

#include <QMutex>

#include <map>
#include <algorithm>

struct IOrderBook
{
    virtual const std::map<Decimal, Decimal> & Bids() const = 0;
    virtual const std::map<Decimal, Decimal> & Asks() const = 0;
    virtual Decimal SeekAmount(const Decimal & priceLow, const Decimal & priceHigh) const = 0;
    virtual Decimal MidPrice() const = 0;

    virtual void AddBid(const Decimal & price, const Decimal & amount) = 0;
    virtual void UpdateBid(const Decimal & price, const Decimal & amount) = 0;
    virtual void AddAsk(const Decimal & price, const Decimal & amount) = 0;
    virtual void UpdateAsk(const Decimal & price, const Decimal & amount) = 0;

protected:
    ~IOrderBook() = default;
};

class OrderBook : private IOrderBook
{
    QMutex mutable mutex;
    std::map<Decimal, Decimal> bids, asks;
    Decimal totBid;
    Decimal totAsk;

public:
    OrderBook() : mutex()
    {}

    template <typename Func> void VisitUnderLock(Func func) const
    {
        QMutexLocker lock(&mutex);
        func(static_cast<const IOrderBook&>(*this));
    }

    template <typename Func> void VisitUnderLock(Func func)
    {
        QMutexLocker lock(&mutex);
        func(static_cast<IOrderBook&>(*this));
    }

private:
    const std::map<Decimal, Decimal> & Bids() const override
    {
        return bids;
    }

    const std::map<Decimal, Decimal> & Asks() const override
    {
        return asks;
    }

    void AddBid(const Decimal & price, const Decimal & amount) override
    {
        bids[price] += amount;
        totBid += amount;
    }

    void UpdateBid(const Decimal & price, const Decimal & amount) override
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

    void AddAsk(const Decimal & price, const Decimal & amount) override
    {
        asks[price] += amount;
        totAsk += amount;
    }

    void UpdateAsk(const Decimal & price, const Decimal & amount) override
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

    Decimal SeekAmount(const Decimal & priceLow, const Decimal & priceHigh) const override
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

    Decimal MidPrice() const override
    {
        auto bidIt = bids.rbegin();
        auto askIt = asks.begin();

        return bidIt != bids.rend() && askIt != asks.end()
            ? (bidIt->first + askIt->first)/2
            : Decimal{};
    }

public:
    void Clear()
    {
        QMutexLocker lock(&mutex);
        bids.clear();
        asks.clear();
    }
};

#endif // ORDERBOOK_H
