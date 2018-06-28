#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gdl.h"
#include "candle.h"
#include "trade.h"
#include "orderbook.h"

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

class MainWindow : public QMainWindow, public GDL::IStreamCallbacks
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<MainWindow>();

    Q_OBJECT

    QSettings * const settings;
    std::unique_ptr<Ui::MainWindow> ui;
    QTimer * const updateTimer;
    GDL::RequestPtr const gdlRequest;
    std::deque<Candle> candles;
    std::deque<Trade> trades;

    OrderBook orderBook;

    // use finer grain and map to GDax::Granularity
    Granularity granularity;

    GDL::StreamPtr gdlStream;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void Shutdown();

private slots: // On...?
    void on_actionE_xit_triggered();
    void Snapshot();
    void Ticker(const Tick & tick);
    void Heartbeat(const QDateTime & serverTime);
    void StateChanged(GDL::ConnectedState state);
    void GranularityChanged(QAction * action);
    void TimerUpdate();

private:
    void AttachExpander(QWidget * parent, QWidget * widget, bool expanded);
    void Connected();
    void GenerateOrderBook();
    void GenerateOrderBookLocked(IOrderBook & lockedOrderBook);
    void GenerateTradeList();

    // <IStreamCallbacks>
    // using interface\callback to allow abstraction instead of emiting QT signals from provider
    // if a callback is not in the UI thread it will need invoking\emiting,
    // currently WebSocket callbacks are all on a worker thread
    void OnSnapshot(const QString & product, const IterableResult<GDL::OrderBookItem> & bids, const IterableResult<GDL::OrderBookItem> & asks) override;
    void OnUpdate(const QString & product, const IterableResult<GDL::OrderBookChange> & changes) override;
    void OnHeartbeat(const QDateTime & serverTime) override;
    void OnTick(const Tick & tick) override;
    void OnStateChanged(GDL::ConnectedState state) override;
    //  </IStreamCallbacks>

    // async request callbacks, are on currently on UI thread
    void OnTrades(const TradesResult & );
    void OnCandles(const CandlesResult & );
};

#endif // MAINWINDOW_H
