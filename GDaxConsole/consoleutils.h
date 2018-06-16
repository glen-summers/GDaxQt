#ifndef CONSOLEUTILS_H
#define CONSOLEUTILS_H

#include <QTime>
#include <QTimer>
#include <QCoreApplication>

#include <future>

namespace SGR
{
    static constexpr const char Black[]  = "\x1b[30m";
    static constexpr const char Red[]    = "\x1b[31m";
    static constexpr const char Green[]  = "\x1b[32m";
    static constexpr const char Yellow[] = "\x1b[33m";
    static constexpr const char Blue[]   = "\x1b[34m";
    static constexpr const char Magenta[]= "\x1b[35m";
    static constexpr const char Cyan[]   = "\x1b[36m";
    static constexpr const char White[]  = "\x1b[37m";

    static constexpr const char BkBlack[]  = "\x1b[30m";
    static constexpr const char BkRed[]    = "\x1b[31m";
    static constexpr const char BkGreen[]  = "\x1b[32m";
    static constexpr const char BkYellow[] = "\x1b[33m";
    static constexpr const char BkBlue[]   = "\x1b[34m";
    static constexpr const char BkMagenta[]= "\x1b[35m";
    static constexpr const char BkCyan[]   = "\x1b[36m";
    static constexpr const char BkWhite[]  = "\x1b[37m";

    static constexpr const char Bold[]   = "\x1b[1m";
    static constexpr const char Faint[]  = "\x1b[2m";
    static constexpr const char Normal[] = "\x1b[22m";
    static constexpr const char Rst[]    = "\x1b[m";
    static constexpr const char RstF[]   = "\x1b[39m";
    static constexpr const char RstB[]   = "\x1b[49m";
}

class ConsoleKeyListener
{
    bool exitFlag = false;
    QTimer exitTimer;
    std::future<void> f;
public:
    ConsoleKeyListener()
    {
        f = std::async(std::launch::async, [this]
        {
            std::getchar();
            exitFlag = true;
        });
        exitTimer.setInterval(1000);
        exitTimer.setSingleShot(false);
        QObject::connect(&exitTimer, &QTimer::timeout, [&]
        {
            if (exitFlag)
            {
                QCoreApplication::quit();
            }
        });
        exitTimer.start();
    }

    static void WaitFor(int seconds)
    {
        QTime endTime = QTime::currentTime().addSecs(seconds);
        while (QTime::currentTime() < endTime)
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
    }

    int Exec() const
    {
        int ret = QCoreApplication::exec();
        f.wait();
        return ret;
    }

};

#endif // CONSOLEUTILS_H