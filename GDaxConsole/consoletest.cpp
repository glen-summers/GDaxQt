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
    constexpr const char product[] ="BTC-EUR";
}

ConsoleTest::ConsoleTest() : request(GDL::GetFactory().CreateRequest(product))
{
    request->SetAuthentication(apiKey, secret, passphrase);

    request->FetchTime().Then([](const ServerTimeResult & result)
    {
        if(result.HasError())
        {
            std::cerr << SGR::Red << SGR::Bold << "Error getting server time: " << result.ErrorString() << SGR::Rst << std::endl;
            return;
        }
        QDateTime dateTime = result();
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

void ConsoleTest::Shutdown()
{
    request.reset();
}
