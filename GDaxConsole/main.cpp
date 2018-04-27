#include <QCoreApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    std::cout << "Qt" << std::endl;

    // event loop
    // return a.exec();

    return 0;
}
