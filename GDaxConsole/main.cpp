
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

    // avoid future time with...
    // can call GET /time
    // {
    // "iso": "2015-01-07T23:47:25.201Z",
    // "epoch": 1420674445.201
    // }
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

// place order
//    provider.PlaceOrder(Decimal("0.01"), Decimal("0.1"), MakerSide::Buy);
// todo listen for web socket update
//    ConsoleKeyListener::WaitFor(5);

//    provider.FetchTime(0, [](TimeResult result)
//    {}

    provider.FetchOrders(0, [](OrderResult result)
    {
        if (result.HasError())
        {
            std::cout << SGR::Red << SGR::Bold << "Error fetching orders : " << result.ErrorString() << SGR::Rst << std::endl;
        }
        else
        {
            for (Order order : result)
            {
                std::cout << SGR::Yellow << order << SGR::Rst << std::endl;
            }
        }
    });

    // cancel orders...
    //provider.CancelOrders();

    return con.Exec();
}
