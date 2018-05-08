#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gdaxlib.h"
#include "restprovider.h"

#include "ui_mainwindow.h"

#include <QMainWindow>
#include <QTimer>
#include <QTextEdit>

#include <memory>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<QTimer> timer;

    GDaxLib g;
    RestProvider restProvider;
    std::vector<Candle> candles;

public:
    explicit MainWindow(QWidget *parent = 0);

public slots:
    void onUpdate();

private slots:
    void on_actionE_xit_triggered();
    void setCandles(std::vector<Candle> candles);

private:
    void generateOrderBook();
    void generateTradeList();

};

#endif // MAINWINDOW_H
