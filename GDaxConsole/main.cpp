#include <QCoreApplication>
#include <iostream>
#include "../GDaxLib/gdaxlib.h"

int main(int argc, char *argv[])
{
    std::cout << "GDax" << std::endl;

    QCoreApplication a(argc, argv);

    GDaxLib g;

    return a.exec();
}
