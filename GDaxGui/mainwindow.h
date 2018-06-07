#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "candle.h"
#include "trade.h"
#include "gdl.h"

#include "flogging.h"

#include <QMainWindow>

#include <memory>
#include <deque>

namespace Ui
{
    class MainWindow;
}
struct Tick;
class QTimer;
class QSettings;

class MainWindow : public QMainWindow, public GDL::Callback
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<MainWindow>();

    Q_OBJECT

    QSettings * const settings;
    std::unique_ptr<Ui::MainWindow> ui;
    QTimer * const updateTimer;
    GDL::InterfacePtr const gdl;
    std::deque<Candle> candles;
    std::deque<Trade> trades;

    // use finer grain and map to GDax::Granularity
    Granularity granularity;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void Shutdown() const;

private slots: // On...?
    void on_actionE_xit_triggered();
    void Snapshot();
    void Candles(std::deque<Candle> values);
    void Trades(std::deque<Trade> values);
    void Ticker(const Tick & tick);
    void Heartbeat(const QDateTime & serverTime);
    void StateChanged(GDL::ConnectedState state);
    void GranularityChanged(QAction * action);
    void TimerUpdate();

private:
    void AttachExpander(QWidget * parent, QWidget * widget, bool expanded);
    void Connected();
    void GenerateOrderBook();
    void GenerateTradeList();

    // Callbacks
    // Now using interface\callback to allow abstraction instead of emiting QT signals from provider
    // if a callback is not in the UI thread it will need invoking\emiting, currently WebSocket callbacks are on
    // a worker thread
    void OnSnapshot() override
    {
        QMetaObject::invokeMethod(this, "Snapshot");
    }

    void OnHeartbeat(const QDateTime & serverTime) override
    {
        QMetaObject::invokeMethod( this, "Heartbeat", Q_ARG( const QDateTime &, serverTime) );
    }

    void OnTick(const Tick & tick) override
    {
        QMetaObject::invokeMethod( this, "Ticker", Q_ARG( const Tick &, tick) );
    }

    void OnStateChanged(GDL::ConnectedState state) override
    {
        QMetaObject::invokeMethod( this, "StateChanged", Q_ARG( GDL::ConnectedState, state) );
    }

    void OnCandles(std::deque<Candle> values) override
    {
        Candles(std::move(values));
    }

    void OnTrades(std::deque<Trade> values) override
    {
        Trades(std::move(values));
        GenerateTradeList();
    }
    // Callbacks
};

#endif // MAINWINDOW_H
