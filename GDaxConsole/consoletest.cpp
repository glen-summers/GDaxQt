#include "consoletest.h"

#include "order.h"
#include "subscription.h"
#include "qfmt.h"

#include "consoleutils.h"

#include "flogging.h"

#include <iostream>

namespace
{
    constexpr const char Product[] ="BTC-EUR";
    Flog::Log flog = Flog::LogManager::GetLog<ConsoleTest>();
}

// "user" channel not getting stream updatres, full does?
ConsoleTest::ConsoleTest(GDL::Auth * auth)
    : request(GDL::GetFactory().CreateRequest(Product, auth))
    , stream(GDL::GetFactory().CreateStream(*this, {{Product}, {Channel::Full, Channel::Matches}}, auth))
{
    request->FetchTime().Then([](const ServerTimeResult & result)
    {
        if(result.HasError())
        {
            std::cerr << SGR::Red << SGR::Bold << "Error getting server time: " << result.ErrorString() << SGR::Rst << std::endl;
            return;
        }
        QDateTime dateTime = result();
        auto now = QDateTime::currentDateTimeUtc();
        auto delta = now.msecsTo(dateTime);
        std::cout << SGR::Cyan
                  << "HB, ServerTime: " << dateTime
                  << ", DeltaMs(-latency): " << delta
                  << SGR::Rst
                  << std::endl;

        flog.Info("DeltaMs(-latency) = {0}", delta);
    });
}

void ConsoleTest::PlaceOrder(const Decimal &size, const Decimal &price, MakerSide side) const
{
    request->PlaceOrder(size, price, side).Then([](OrderResult result)
    {
        if(result.HasError())
        {
            std::cerr << SGR::Red << SGR::Bold << "Error placing order : " << result.ErrorString() << SGR::Rst << std::endl;
            return;
        }
        std::cout << SGR::Green << result() << SGR::Rst << std::endl;
    });
}

void ConsoleTest::Orders() const
{
    request->FetchOrders().Then([](OrdersResult result)
    {
        if (result.HasError())
        {
            std::cout << SGR::Red << SGR::Bold << "Error fetching orders : " << result.ErrorString() << SGR::Rst << std::endl;
            return;
        }
        for (Order order : result)
        {
            std::cout << SGR::Yellow << order << SGR::Rst << std::endl;
        }
    });
}

void ConsoleTest::CancelOrders() const
{
    request->CancelOrders().Then([](CancelOrdersResult result)
    {
        if (result.HasError())
        {
            std::cout << SGR::Red << SGR::Bold << "Error cancelling orders : " << result.ErrorString() << SGR::Rst << std::endl;
            return;
        }
        for (auto orderId : result)
        {
            std::cout << "Cancelled: " << SGR::Yellow << orderId << SGR::Rst << std::endl;
        }
    });
}

void ConsoleTest::OnSnapshot(const QString & product, const IterableResult<GDL::OrderBookItem> &bids, const IterableResult<GDL::OrderBookItem> &asks)
{

}

void ConsoleTest::OnUpdate(const QString &product, const IterableResult<GDL::OrderBookChange> &changes)
{

}

void ConsoleTest::OnHeartbeat(const QDateTime &serverTime)
{

}

void ConsoleTest::OnTick(const Tick &tick)
{

}

void ConsoleTest::OnStateChanged(GDL::ConnectedState state)
{

}
