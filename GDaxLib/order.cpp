#include "order.h"

#include "formatter.h"
#include "qfmt.h"

#include <QJsonObject>

#include <unordered_map>

namespace
{
    constexpr const char Limit[] = "limit";
    constexpr const char Market[] = "market";

    constexpr const char Pending[] = "pending";
    constexpr const char Active[] = "active";
    constexpr const char Rejected[] = "rejected";
    constexpr const char Open[] = "open";
    constexpr const char Done[] = "done";
    constexpr const char Settled[] = "settled";

    constexpr const char DecrementAndCancel[] = "dc";
    constexpr const char CancelOldest[] = "co";
    constexpr const char CancelNewest[] = "cn";
    constexpr const char CancelBoth[] = "cb";
}

OrderType OrderTypeFromString(const std::string & value)
{
    return value == Limit
            ? OrderType::Limit
            : value == Market
              ? OrderType::Market
              : throw std::runtime_error("Unknown value: '" + value + "'");
}

std::ostream & operator << (std::ostream & s, OrderType orderType)
{
    switch (orderType)
    {
        case OrderType::Limit: return s << Limit;
        case OrderType::Market: return s << Market;
        default: throw std::runtime_error("Invalid type");
    }
}

SelfTradeProtection SelfTradeProtectionFromString(const std::string & value)
{
    static std::unordered_map<std::string, SelfTradeProtection> lookup
    {
        {DecrementAndCancel, SelfTradeProtection::DecrementAndCancel},
        {CancelOldest, SelfTradeProtection::CancelOldest},
        {CancelNewest, SelfTradeProtection::CancelNewest},
        {CancelBoth, SelfTradeProtection::CancelBoth}
    };

    auto it = lookup.find(value.c_str());
    return it!=lookup.end() ? it->second : throw std::runtime_error("Unknown value: '" + value + "'");
}

OrderStatus OrderStatusFromString(const std::string & value)
{
    static std::unordered_map<std::string, OrderStatus> lookup
    {
        {Pending, OrderStatus::Pending},
        {Active, OrderStatus::Active},
        {Rejected, OrderStatus::Rejected},
        {Open, OrderStatus::Open},
        {Done, OrderStatus::Done},
        {Settled, OrderStatus::Settled}
    };

    auto it = lookup.find(value.c_str());
    return it!=lookup.end() ? it->second : throw std::runtime_error("Unknown value: '" + value + "'");
}

std::ostream & operator << (std::ostream & s, OrderStatus status)
{
    switch (status)
    {
        case OrderStatus::Pending: return s << Pending;
        case OrderStatus::Active:  return s << Active;
        case OrderStatus::Rejected: return s << Rejected;
        case OrderStatus::Open: return s << Open;
        case OrderStatus::Done: return s << Done;
        case OrderStatus::Settled: return s << Settled;
        default: throw std::runtime_error("Invalid status");
    }
}

Order Order::FromJson(const QJsonValue & value)
{
    auto object = value.toObject();
    return Order
    {
        object["id"].toString(),
        Decimal(object["price"].toString().toStdString()),
        Decimal(object["size"].toString().toStdString()),
        object["product_id"].toString(),
        MakerSideFromString(object["side"].toString().toStdString()),
        SelfTradeProtection::None, //SelfTradeProtectionFromString(object["stp"].toString().toStdString()), // optional
        OrderTypeFromString(object["type"].toString().toStdString()),
        //"time_in_force" : GTC(def), GTT, IOC, FOK
        false, //bool PostOnly;
        QDateTime::fromString(object["created_at"].toString(), Qt::ISODateWithMs),
        //"fill_fees": "0.0000000000000000" prec?
        //"filled_size": "0.00000000", prec?
        //"executed_value": "0.0000000000000000"//prec?
        OrderStatusFromString(object["status"].toString().toStdString()),
        false//bool settled;
    };
}

std::ostream & operator <<(std::ostream & s, SelfTradeProtection stp)
{
    switch (stp)
    {
        case SelfTradeProtection::None:
            return s;
        case SelfTradeProtection::DecrementAndCancel:
            return s << DecrementAndCancel;
        case SelfTradeProtection::CancelOldest:
            return s << CancelOldest;
        case SelfTradeProtection::CancelNewest:
            return s << CancelNewest;
        case SelfTradeProtection::CancelBoth:
            return s << CancelBoth;

        default:
            throw std::runtime_error("Invalid stp");
    }
}

std::ostream & operator <<(std::ostream & s, const Order & order)
{
    return Formatter::Format(s, "id:{0}, price:{1}, size:{2}, prd:{3}, side:{4}, stp:{5}, type:{6}, created:{7}, status:{8}",
                             order.id, order.price, order.size, order.productId, MakerSideToString(order.side),
                             order.stp, order.type, order.createdAt.toString(Qt::DateFormat::ISODateWithMs), order.status);
}
