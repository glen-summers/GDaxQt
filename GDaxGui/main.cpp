#include "mainwindow.h"

int main(int argc, char *argv[])
{
    try
    {
        QGuiApplication app(argc, argv);
        MainWindow window;
        window.showMaximized();

        return app.exec();
    }
    catch (const std::exception & e)
    {
        qWarning((std::string("Error: ") + e.what()).c_str());
    }
}
