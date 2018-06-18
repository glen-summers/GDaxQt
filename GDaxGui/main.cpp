#include "mainwindow.h"

#include "gdl.h"
#include "utils.h"
#include "gdaxprovider.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QLibrary>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //QApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);
    //QApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);

// cld set dpi aware and set scales appropriately to fix win32 rouch dpi bug...
//#ifdef _WIN32
//  {
//    typedef unsigned short (*SetProcessDpiAwarenessT)(int value);
//    QLibrary user32("user32.dll", NULL);
//    SetProcessDpiAwarenessT SetProcessDpiAwarenessD = (SetProcessDpiAwarenessT)user32.resolve("SetProcessDpiAwarenessInternal");
//    if (SetProcessDpiAwarenessD)
//    {
//        SetProcessDpiAwarenessD(1); //PROCESS_PER_MONITOR_DPI_AWARE
//    }
//  }
//#endif

    QApplication a(argc, argv);

    QApplication::setStyle("Fusion");
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(darkPalette);

    QSurfaceFormat fmt;
    fmt.setSamples(4);
    QSurfaceFormat::setDefaultFormat(fmt);

    Flog::LogManager::SetLevel(Flog::Level::Info);
    Flog::LogManager::SetThreadName("Ui");

    if (argc>1 && strcmp(argv[1], "-sandbox")==0)
    {
        GDL::SetFactory([](GDL::Callback & callback)
        {
            return GDL::InterfacePtr(Utils::QMake<GDaxProvider>("GDaxProvider",
                "wss://ws-feed-public.sandbox.gdax.com",
                "https://api-public.sandbox.gdax.com",
                callback));
        });
    }

    MainWindow w;
    w.showMaximized();

    int ret = a.exec();
    w.Shutdown();
    return ret;
}
