
#include "consoletest.h"

#include "utils.h"
#include "consoleutils.h"

#include "defs.h" // for makerSide
#include "factory.h"

#include "flogging.h"

#include <QCoreApplication>

#include <iostream>

using namespace std::chrono_literals;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Flog::LogManager::SetLevel(Flog::Level::Info);
    Flog::LogManager::SetThreadName("Main");

    EnableAnsiConsole();
    std::cout << SGR::Bold << "GDax" << SGR::Rst << std::endl;

    ConsoleKeyListener con;

//    RestProvider restProvider;
//    QDateTime start = QDateTime::currentDateTimeUtc().addDays(-1);
//    QDateTime end = QDateTime::currentDateTimeUtc().addSecs(-60);// avoid future value, causes full dl
//    restProvider.FetchCandles(start, end, Granularity::Hours);

    GDL::SetFactory(std::make_unique<GDL::Factory>("wss://ws-feed-public.sandbox.gdax.com", "https://api-public.sandbox.gdax.com"));

    ConsoleTest test;

//    auto now = QDateTime::currentDateTimeUtc();
//    provider.FetchTime([&](ServerTimeResult result)
//    {
//        if (result.HasError())
//        {
//            std::cout << SGR::Red << SGR::Bold << "Error fetching time : " << result.ErrorString() << SGR::Rst << std::endl;
//            return;
//        }
//        std::cout << "ServerTime: " << result().toString().toStdString()
//                  << ", DeltaMs(+latency): " << now.msecsTo(result())
//                  << std::endl;
//    });

    // still borked itf? we need to have lamdba here, and repeatedly pass around
    // better to model like std::future and use as return value
    // but get() syncronous wait may be not be ideal for qt
    // but can implement a Then([](){}); if truely async need to handle case already completed
    test.PlaceOrder(Decimal("0.01"), Decimal("0.1"), MakerSide::Buy);
    // todo have web socket listen for update + add our order id
    ConsoleKeyListener::WaitFor(5);

    test.Orders();

    ConsoleKeyListener::WaitFor(5);

    test.CancelOrders();

    ConsoleKeyListener::WaitFor(5);

    auto ret = con.Exec();

    test.Shutdown();
    // Shutdown causes: QIODevice::write (QTcpSocket): device not open
    // and WebSocketStream : SocketError: unknown(-1)
    // not seen in gui shutdown

    Utils::DumpObjects();
    return ret;
}
