
#include "websocketstream.h"
#include "restprovider.h"
#include "authenticator.h"
#include "utils.h"

#include <QCoreApplication>
#include <QTimer>
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
    std::cout << "GDax" << std::endl;

//    wprintf(L"\x1b[31mThis text has a red foreground using SGR.31.\r\n");
//    wprintf(L"\x1b[1mThis text has a bright (bold) red foreground using SGR.1 to affect the previous color setting.\r\n");
//    wprintf(L"\x1b[mThis text has returned to default colors using SGR.0 implicitly.\r\n");
//    wprintf(L"\x1b[34;46mThis text shows the foreground and background change at the same time.\r\n");
//    wprintf(L"\x1b[0mThis text has returned to default colors using SGR.0 explicitly.\r\n");
//    wprintf(L"\x1b[31;32;33;34;35;36;101;102;103;104;105;106;107mThis text attempts to apply many colors in the same command. Note the colors are applied from left to right so only the right-most option of foreground cyan (SGR.36) and background bright white (SGR.107) is effective.\r\n");
//    wprintf(L"\x1b[39mThis text has restored the foreground color only.\r\n");
//    wprintf(L"\x1b[49mThis text has restored the background color only.\r\n");

    QCoreApplication a(argc, argv);

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

    bool exitFlag = false;
    auto f = std::async(std::launch::async, [&]
    {
        std::getchar();
        exitFlag = true;
    });

    QTimer exitTimer;
    exitTimer.setInterval(1000);
    exitTimer.setSingleShot(false);
    QObject::connect(&exitTimer, &QTimer::timeout, [&]
    {
        if (exitFlag)
        {
            a.quit();
        }
    });
    exitTimer.start();

    // sandbox test key
    auto apiKey = QByteArrayLiteral("86feb99f0b2244a1b756c9aca9c8eb0c");
    auto secret = QByteArrayLiteral("T9e1Aw7BFB0PJPKqd8VtMDH6agezkEBESWYrJHEoReS2KTgV7zIhDSSnnl5Bc5AqlswSz1rKam080937FTIQWA==");
    auto passphrase = QByteArrayLiteral("eoxq18akv3u");

    RestProvider provider("https://api-public.sandbox.gdax.com", new QNetworkAccessManager());
    provider.SetAuthenticator(new Authenticator(std::move(apiKey), QByteArray::fromBase64(std::move(secret)), std::move(passphrase)));

// place order
//    auto size = DecNs::fromString<Decimal>("0.01");
//    auto price = DecNs::fromString<Decimal>("0.1");
//    auto side = MakerSide::Buy;
//    provider.PlaceOrder(size, price, side);

// todo listen for web socket update
//    QTime endTime = QTime::currentTime().addSecs(5);
//    while (QTime::currentTime() < endTime)
//    {
//        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
//    }

    provider.FetchOrders(0, [](OrderResult result)
    {
        if (result.Error() != QNetworkReply::NoError)
        {
            std::cout << "Error fetching orders : " << result.ErrorString() << std::endl;
        }
        else
        {
            for (Order order : result)
            {
                std::cout << order << std::endl;
            }
        }
    });

    // cancel orders...
    //provider.CancelOrders();

    int ret = a.exec();
    f.wait();
    return ret;
}
