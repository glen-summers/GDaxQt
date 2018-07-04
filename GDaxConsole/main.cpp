
#include "consoletest.h"

#include "utils.h"
#include "consoleutils.h"

#include "defs.h" // for makerSide
#include "factory.h"

#include "flogging.h"

#include <QCoreApplication>

#include <iostream>

using namespace std::chrono_literals;

namespace
{
    // sandbox test key settings
    constexpr const char ApiKey[] = "86feb99f0b2244a1b756c9aca9c8eb0c";
    constexpr const char Secret[] = "T9e1Aw7BFB0PJPKqd8VtMDH6agezkEBESWYrJHEoReS2KTgV7zIhDSSnnl5Bc5AqlswSz1rKam080937FTIQWA==";
    constexpr const char Passphrase[] = "eoxq18akv3u";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Flog::LogManager::SetLevel(Flog::Level::Debug);
    Flog::LogManager::SetThreadName("Main");
    GDL::SetSandbox();

    EnableAnsiConsole();
    std::cout << SGR::Bold << "GDax" << SGR::Rst << std::endl;

    ConsoleKeyListener con;

//    RestProvider restProvider;
//    QDateTime start = QDateTime::currentDateTimeUtc().addDays(-1);
//    QDateTime end = QDateTime::currentDateTimeUtc().addSecs(-60);// avoid future value, causes full dl
//    restProvider.FetchCandles(start, end, Granularity::Hours);

    {//scope
        GDL::Auth auth { ApiKey, Secret, Passphrase };
        ConsoleTest test(&auth);

        // stream connect is async, need to wait until stream.State == Connected to ensure get updates
        // need a Connect().Then([](){ do stuff })?
        // wait for server time result, todo have a WaitForResult version
        ConsoleKeyListener::WaitFor(5);

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
    for(int i=0;i<10;++i)
    {
        test.PlaceOrder(Decimal("0.01"), Decimal("0.1"), MakerSide::Buy);
    }
    // todo have web socket listen for update + add our order id
    ConsoleKeyListener::WaitFor(5);

    test.Orders();

    ConsoleKeyListener::WaitFor(5);

    test.CancelOrders();

    ConsoleKeyListener::WaitFor(5);
    } // scope


    auto ret = con.Exec();

    // Shutdown caused: QIODevice::write (QTcpSocket): device not open
    // and WebSocketStream : SocketError: unknown(-1)
    // not seen in gui shutdown

    Utils::DumpObjects();
    return ret;
}
