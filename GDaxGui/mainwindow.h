#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "candle.h"
#include "trade.h"

#include "flogging.h"

#include <QMainWindow>

#include <memory>
#include <deque>

namespace Ui
{
    class MainWindow;
}
struct Tick;
class GDaxLib;
class RestProvider;
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
    RestProvider * const restProvider;
    std::deque<Candle> candles;
    std::deque<Trade> trades;

    // use finer grain and map to GDax::Granularity
    Granularity granularity;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void Shutdown() const;

public slots:
    void Update();

private slots:
    void on_actionE_xit_triggered();
    void Candles(std::deque<Candle> values);
    void Trades(std::deque<Trade> values);
    void Ticker(const Tick & tick);
    void Heartbeat(const QDateTime & serverTime);
    void StateChanged(ConnectedState state);
    void GranularityChanged(QAction * action);

private:
    void AttachExpander(QWidget * parent, QWidget * widget, bool expanded);
    void Connected();
    void GenerateOrderBook();
    void GenerateTradeList();
};

#endif // MAINWINDOW_H
