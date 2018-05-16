#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gdaxlib.h"
#include "restprovider.h"

#include "ui_mainwindow.h"

#include <QMainWindow>
#include <QTimer>
#include <QTextEdit>
#include <QComboBox>

#include <memory>

class MainWindow : public QMainWindow
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<MainWindow>();

    Q_OBJECT

    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<QTimer> timer;

    GDaxLib gDaxLib;
    RestProvider restProvider;
    std::deque<Candle> candles;
    std::deque<Trade> trades;

    // use finer grain and map to GDax::Granularity
    Granularity granularity = Granularity::Hours;

public:
    explicit MainWindow(QWidget *parent = 0);

public slots:
    void Update();

private slots:
    void on_actionE_xit_triggered();
    void Candles(std::deque<Candle> values);
    void Trades(std::deque<Trade> values);
    void Ticker(Tick tick);
    void StateChanged(GDaxLib::State state);
    void GranularityChanged(QAction *);

private:
    void Connected();
    void GenerateOrderBook();
    void GenerateTradeList();
};

#endif // MAINWINDOW_H
