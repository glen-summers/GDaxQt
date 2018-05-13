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
    inline static const Flog::Log log = Flog::LogManager::GetLog<MainWindow>();

    Q_OBJECT
    typedef std::deque<Tick> TicksType;

    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<QTimer> timer;

    GDaxLib g;
    RestProvider restProvider;
    std::vector<Candle> candles;
    TicksType ticks;

public:
    explicit MainWindow(QWidget *parent = 0);

public slots:
    void onUpdate();

private slots:
    void on_actionE_xit_triggered();
    void setCandles(std::vector<Candle> candles);
    void setTrades(std::vector<Trade> trades);
    void onTick(Tick tick);

private:
    void generateOrderBook();
    void generateTradeList();
};

#endif // MAINWINDOW_H
