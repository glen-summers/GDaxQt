#include "mainwindow.h"
#include <QPainter.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
{
    ui->setupUi(this);
}

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 3));
    auto rc  = painter.window();
    auto e = 10;
    rc.adjust(e,e,-e,-e);
    painter.drawRect(rc);

    QFont font = painter.font();
    font.setPointSize(12);
    painter.setFont(font);
    painter.drawText(rc, "Hello Qt");
//    qInfo("paint");
}
