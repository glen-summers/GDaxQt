#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gdaxlib.h"
#include "restprovider.h"

#include "ui_mainwindow.h"

#include <QMainWindow>

#include <memory>

class QTimer;
class QSettings;

class MainWindow : public QMainWindow
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<MainWindow>();

    Q_OBJECT

    QSettings * const settings;
    std::unique_ptr<Ui::MainWindow> ui;
    QTimer * const timer;
    GDaxLib * const gDaxLib;
    QThread * const workerThread;
    RestProvider restProvider;
    std::deque<Candle> candles;
    std::deque<Trade> trades;

    // use finer grain and map to GDax::Granularity
    Granularity granularity;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void Shutdown() const;

public slots:
    void Update();

private slots:
    void on_actionE_xit_triggered();
    void Candles(std::deque<Candle> values);
    void Trades(std::deque<Trade> values);
    void Ticker(Tick tick);
    void Heartbeat(const QDateTime & serverTime);
    void StateChanged(GDaxLib::State state);
    void GranularityChanged(QAction *);

private:
    void AttachExpander(QWidget * parent, QWidget * widget, bool expanded);
    void Connected();
    void GenerateOrderBook();
    void GenerateTradeList();
};

#endif // MAINWINDOW_H
