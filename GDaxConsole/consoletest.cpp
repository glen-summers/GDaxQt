#include "consoletest.h"

#include "order.h"
#include "qfmt.h"

#include "consoleutils.h"

#include <iostream>

namespace
{
    // sandbox test key settings
    constexpr const char apiKey[] = "86feb99f0b2244a1b756c9aca9c8eb0c";
    constexpr const char secret[] = "T9e1Aw7BFB0PJPKqd8VtMDH6agezkEBESWYrJHEoReS2KTgV7zIhDSSnnl5Bc5AqlswSz1rKam080937FTIQWA==";
    constexpr const char passphrase[] = "eoxq18akv3u";
}

ConsoleTest::ConsoleTest() : gdl(GDL::Create(*this))
{
    gdl->SetAuthentication(apiKey, secret, passphrase);

    gdl->FetchTime().Then([](const ServerTimeResult & serverTime)
    {
        QDateTime dateTime = serverTime(); // catch here
        auto now = QDateTime::currentDateTimeUtc();
        std::cout << SGR::Cyan
                  << "HB, ServerTime: " << dateTime
                  << ", DeltaMs(-latency): " << now.msecsTo(dateTime)
                  << SGR::Rst
                  << std::endl;
    });
}

void ConsoleTest::PlaceOrder(const Decimal &size, const Decimal &price, MakerSide side) const
{
    gdl->PlaceOrder([](OrderResult result)
    {
        if(result.HasError())
        {
            std::cerr << SGR::Red << SGR::Bold << "Error placing order : " << result.ErrorString() << SGR::Rst << std::endl;
            return;
        }
        std::cout << SGR::Green << result() << SGR::Rst << std::endl;
    }, size, price, side);
}

void ConsoleTest::Orders() const
{
    auto ordersFn = [](OrdersResult result)
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
    };
    gdl->FetchOrders(ordersFn);
}

void ConsoleTest::CancelOrders() const
{
    auto fn = [](CancelOrdersResult result)
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
    };

    gdl->CancelOrders(fn);
}

void ConsoleTest::Shutdown()
{
    gdl->Shutdown();
    gdl.reset();
}

void ConsoleTest::OnSnapshot()
{
    std::cout << "Snapshot" << std::endl;
}

void ConsoleTest::OnHeartbeat(const QDateTime & serverTime)
{
    auto now = QDateTime::currentDateTimeUtc();
    std::cout << SGR::Cyan
              << "HB, ServerTime: " << serverTime
              << ", DeltaMs(-latency): " << now.msecsTo(serverTime)
              << SGR::Rst
              << std::endl;
}

void ConsoleTest::OnTick(const Tick & /*tick*/)
{
    std::cout << SGR::Magenta << "tick " << SGR::Rst << std::endl;
}

void ConsoleTest::OnStateChanged(GDL::ConnectedState state)
{
    std::cout << "state: " << (unsigned)state << std::endl;
}

void ConsoleTest::OnCandles(const CandlesResult & value)
{
    std::cout << "candles: " << (value.HasError() ? value.ErrorString() : std::to_string(value.count()))
                                                   << std::endl;
}

void ConsoleTest::OnTrades(const TradesResult & value)
{
    std::cout << "trades: " << (value.HasError() ? value.ErrorString() : std::to_string(value.count()))
                                                  << std::endl;
}
