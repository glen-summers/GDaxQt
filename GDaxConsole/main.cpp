
#include "consoleutils.h"
#include "websocketstream.h"
#include "restprovider.h"
#include "authenticator.h"
#include "utils.h"

#include <QCoreApplication>

#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStringBuilder>
#include <QMessageAuthenticationCode>

#include <iostream>

using namespace std::chrono_literals;

int main(int argc, char *argv[])
{
    Utils::EnableAnsiConsole();
    std::cout << SGR::Bold << "GDax" << SGR::Rst << std::endl;

    QCoreApplication a(argc, argv);
    ConsoleKeyListener con;

//    RestProvider restProvider;
//    QDateTime start = QDateTime::currentDateTimeUtc().addDays(-1);
//    QDateTime end = QDateTime::currentDateTimeUtc().addSecs(-60);// avoid future value, causes full dl
//    restProvider.FetchCandles(start, end, Granularity::Hours);

    // sandbox test key
    auto apiKey = QByteArrayLiteral("86feb99f0b2244a1b756c9aca9c8eb0c");
    auto secret = QByteArrayLiteral("T9e1Aw7BFB0PJPKqd8VtMDH6agezkEBESWYrJHEoReS2KTgV7zIhDSSnnl5Bc5AqlswSz1rKam080937FTIQWA==");
    auto passphrase = QByteArrayLiteral("eoxq18akv3u");

    RestProvider provider("https://api-public.sandbox.gdax.com", new QNetworkAccessManager());
    provider.SetAuthenticator(new Authenticator(std::move(apiKey), QByteArray::fromBase64(std::move(secret)), std::move(passphrase)));

    auto now = QDateTime::currentDateTimeUtc();
    provider.FetchTime([&](ServerTimeResult result)
    {
        if (result.HasError())
        {
            std::cout << SGR::Red << SGR::Bold << "Error fetching time : " << result.ErrorString() << SGR::Rst << std::endl;
            return;
        }
        std::cout << "ServerTime: " << result.Value().toString().toStdString()
                  << ", DeltaMs(+latency): " << now.msecsTo(result.Value())
                  << std::endl;
    });


// place order
//    provider.PlaceOrder(Decimal("0.01"), Decimal("0.1"), MakerSide::Buy);
// todo listen for web socket update
//    ConsoleKeyListener::WaitFor(5);

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
    provider.FetchOrders(ordersFn, 0);

    // cancel orders...
    //provider.CancelOrders();

    return con.Exec();
}
