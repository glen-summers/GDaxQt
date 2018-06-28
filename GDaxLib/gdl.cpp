#include "gdl.h"
#include "factory.h"

#include "websocketstream.h"
#include "restprovider.h"

namespace
{
    const char StreamUrl[] = "wss://ws-feed.pro.coinbase.com";
    const char RequestUrl[] = "https://api.pro.coinbase.com";
    const char SandboxStreamUrl[] = "wss://ws-feed-public.sandbox.gdax.com"; // pro ver?
    const char SandboxRequestUrl[] = "https://api-public.sandbox.gdax.com"; // pro ver?

    GDL::FactoryPtr factory = std::make_unique<GDL::Factory>(StreamUrl, RequestUrl);
}

void GDL::SetFactory(GDL::FactoryPtr value)
{
    factory = std::move(value);
}

const GDL::IFactory & GDL::GetFactory()
{
    return *factory;
}

void GDL::SetSandbox()
{
    SetFactory(std::make_unique<GDL::Factory>(SandboxStreamUrl, SandboxRequestUrl));
}

QString JsonValueToString::FromJson(const QJsonValue &value)
{
    return value.toString();
}

QDateTime ServerTimeToDateTime::FromJson(const QJsonValue &value)
{
    return QDateTime::fromString(value["iso"].toString(), Qt::DateFormat::ISODateWithMs);
}

GDL::OrderBookItem GDL::OrderBookItem::FromJson(const QJsonValue &value)
{
    const auto & ar = value.toArray();
    return
    {
        Decimal(ar[0].toString().toStdString()), Decimal(ar[1].toString().toStdString())
    };
}

GDL::OrderBookChange GDL::OrderBookChange::FromJson(const QJsonValue &value)
{
    const auto & ar = value.toArray();
    return
    {
        MakerSideFromString(ar[0].toString().toStdString()),
                Decimal(ar[1].toString().toStdString()),
                Decimal(ar[2].toString().toStdString())
    };
}
