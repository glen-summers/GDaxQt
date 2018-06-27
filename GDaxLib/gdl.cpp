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
