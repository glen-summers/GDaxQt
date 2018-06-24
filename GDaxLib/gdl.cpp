#include "gdl.h"
#include "factory.h"

#include "websocketstream.h"
#include "restprovider.h"

namespace
{
    GDL::FactoryPtr factory = std::make_unique<GDL::Factory>("wss://ws-feed.gdax.com", "https://api.gdax.com");
}

void GDL::SetFactory(GDL::FactoryPtr value)
{
    factory = std::move(value);
}

const GDL::IFactory & GDL::GetFactory()
{
    return *factory;
}
