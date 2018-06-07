#include "mainwindow.h"

#include "tick.h"
#include "depthchart.h"
#include "candleoverlay.h"
#include "orderbook.h"

#include "utils.h"
#include "expandbutton.h"

#include "ui_mainwindow.h"

#include <QTimer>
#include <QToolBar>
#include <QActionGroup>
#include <QThread>
#include <QSettings>
#include <QTextStream>
#include <QMutex>

namespace
{
    constexpr int UpdateTimerMs = 5000;

    void SetupActionGroup(QActionGroup & group, const std::initializer_list<std::pair<QAction &, Granularity>> & actions, QAction & selected)
    {
        for (const auto & action : actions)
        {
            action.first.setData(static_cast<unsigned int>(action.second));
            group.addAction(&action.first);
        }
        selected.setChecked(true);
    }

    constexpr const char * TradesVisibleSetting = "TradesVisible";
    constexpr const char * OrdersVisibleSetting = "OrdersVisible";
    constexpr const char * DepthVisibleSetting = "DepthVisible";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , settings(Utils::QMake<QSettings>("settings", "Crapola", nullptr, this))
    , ui(std::make_unique<Ui::MainWindow>())
    , updateTimer(Utils::QMake<QTimer>("updateTimer", this))
    , gdl(GDL::Create(*this))
    , granularity()
{
    ui->setupUi(this);

    AttachExpander(ui->centralWidget, ui->trades, settings->value(TradesVisibleSetting, true).toBool());
    AttachExpander(ui->centralWidget, ui->orderBook, settings->value(OrdersVisibleSetting, true).toBool());
    AttachExpander(ui->centralWidget, ui->depthChart, settings->value(DepthVisibleSetting, true).toBool());

    granularity = Granularity::Hours; // persist
    SetupActionGroup(*new QActionGroup(this),
    {
        {*ui->action1M, Granularity::Minutes},
        {*ui->action5M, Granularity::FiveMinutes},
        {*ui->action15M, Granularity::FifteenMinutes},
        {*ui->action1H, Granularity::Hours},
        {*ui->action6H, Granularity::SixHours},
        {*ui->action1D, Granularity::Days}
    }, *ui->action1H);
    connect(ui->menuGranularity, &QMenu::triggered, this, &MainWindow::GranularityChanged);

    ui->depthChart->SetProvider(gdl.get());

    connect(updateTimer, &QTimer::timeout, this, &MainWindow::TimerUpdate);
    updateTimer->start(UpdateTimerMs);

    Utils::QMake<CandleOverlay>("CandleOverlay", *ui->candleChart);
}

MainWindow::~MainWindow() = default;

void MainWindow::AttachExpander(QWidget * parent, QWidget * widget, bool expanded)
{
    auto layout = (QBoxLayout*)parent->layout();
    int index = layout->indexOf(widget);
    auto button = Utils::QMake<ExpandButton>("ExpandButton", parent);
    button->setControlled(widget);
    layout->insertWidget(index, button);
    if (!expanded)
    {
        button->toggle();
    }
}

void MainWindow::Shutdown() const
{
    settings->setValue(TradesVisibleSetting, !ui->trades->isHidden());
    settings->setValue(OrdersVisibleSetting, !ui->orderBook->isHidden());
    settings->setValue(DepthVisibleSetting, !ui->depthChart->isHidden());

    gdl->Shutdown();
}

void MainWindow::TimerUpdate()
{
    Flog::ScopeLog s(log, Flog::Level::Info, "TimerUpdate");

    ui->depthChart->update();
    ui->candleChart->update();
    GenerateOrderBook();
    GenerateTradeList();
}

void MainWindow::on_actionE_xit_triggered()
{
    QApplication::quit();
}

void MainWindow::Snapshot()
{
    Flog::ScopeLog s(log, Flog::Level::Info, "Snapshot");
    ui->depthChart->update();
    GenerateOrderBook();
}

void MainWindow::GenerateOrderBook()
{
    auto & orderBookUi = *ui->orderBook;
    if (orderBookUi.isHidden())
    {
        orderBookUi.document()->clear();
        return;
    }

    Flog::ScopeLog s(log, Flog::Level::Info, "GenerateOrderBook");

    QFont font = orderBookUi.document()->defaultFont();
    QFontMetrics fm(font);
    int fontHeight = fm.height();
    int lines = orderBookUi.height()/2/fontHeight-1;

    // lock orderbook, move\improve impl
    const auto & orderBook = gdl->Orders();
    QMutexLocker lock(&const_cast<QMutex&>(orderBook.Mutex()));

    const auto & asks = orderBook.Asks();
    const auto & bids = orderBook.Bids();
    int priceDecs = 2;
    int amountDecs = 4;
    Decimal tot;

    int count = lines;

    // just use std::ostringstream?
    QString prevPrice, prevAmount, prevTotAmount;
    QString html;
    QTextStream stream(&html); // perf test?

    // subset of html - http://doc.qt.io/qt-5/richtext-html-subset.html
    stream << R"(<style>
td { text-align:right; }
td.down { color:red; }
td.down span { color:darkred; }
td.up { color:limegreen; }
td.up span { color:darkgreen; }
td.mid { color:white; padding: 10px; }
td.amount { color:white; }
td.amount span { color:grey; }
</style>
<table width="100%" cellspacing="0" cellpadding="0">)";

    QStringList reverso;
    reverso.reserve(count);
    for (auto it = asks.begin(); count != 0 && it != asks.end(); ++it, --count)
    {
        auto p = it->first;
        auto a = it->second;
        tot += a;
        // use toString and truncate to avoid tmp double?
        QString price = QString::number(p.getAsDouble(), 'f', priceDecs);
        QString amount = QString::number(a.getAsDouble(), 'f', amountDecs);
        QString totAmount = QString::number(tot.getAsDouble(), 'f', amountDecs);

        reverso << QString(R"(<tr><td class="down">%1</td><td class="amount">%2</td><td class="amount">%3</td><\tr>)")
                   .arg(Utils::DiffText(prevPrice, price))
                   .arg(Utils::DiffText(prevAmount, amount))
                   .arg(Utils::DiffText(prevTotAmount, totAmount));

        prevPrice = price;
        prevAmount = amount;
        prevTotAmount = totAmount;
    }

    for (auto it = reverso.rbegin(); it!=reverso.rend(); ++it)
    {
        stream << *it;
    }
    stream << "</table>";

    // add mid\last trade\spread?
    stream << R"(<table width="100%" cellspacing="0" cellpadding="0">
<tr><td class="mid"></td></tr>
</table>)";

    stream << R"(<table width="100%" cellspacing="0" cellpadding="0">)";

    count = lines;
    tot = 0;
    prevPrice.clear();
    prevAmount.clear();
    prevTotAmount.clear();
    for (auto it = bids.rbegin(); count != 0 && it != bids.rend(); ++it, --count)
    {
        auto p = it->first;
        auto a = it->second;
        tot += a;
        // use toString and truncate to avoid tmp double?
        QString price = QString::number(p.getAsDouble(), 'f', priceDecs);
        QString amount = QString::number(a.getAsDouble(), 'f', amountDecs);
        QString totAmount = QString::number(tot.getAsDouble(), 'f', amountDecs);

        stream << "<tr>"
               << "<td class=\"up\">" << Utils::DiffText(prevPrice, price) << "</td>"
               << "<td class=\"amount\">" << Utils::DiffText(prevAmount, amount) << "</td>"
               << "<td class=\"amount\">" << Utils::DiffText(prevTotAmount, totAmount) << "</td>"
               << "<\tr>";
        prevPrice = price;
        prevAmount = amount;
        prevTotAmount = totAmount;
   }

   stream << "</table>";
   orderBookUi.document()->setHtml(*stream.string());
}

void MainWindow::GenerateTradeList()
{
    auto & tradesWidget = *ui->trades;
    if (tradesWidget.isHidden())
    {
        tradesWidget.document()->clear();
        return;
    }

    Flog::ScopeLog s(log, Flog::Level::Info, "GenerateTradeList");

    QFont font = tradesWidget.document()->defaultFont();
    QFontMetrics fm(font);
    int fontHeight = fm.height();
    int count = tradesWidget.height()/fontHeight;

    int priceDecs = 2;
    int amountDecs = 4;

    // just use std::ostringstream?
    QString prevPrice, prevAmount, prevTime;
    QString html;
    QTextStream stream(&html); // perf test?

    // subset of html - http://doc.qt.io/qt-5/richtext-html-subset.html
    stream << R"(<style>
td { text-align:right; }
td.down { color:red; }
td.down span { color:darkred; }
td.up { color:limegreen; }
td.up span { color:darkgreen; }
td.mid { color:white; padding: 10px; }
td.amount { color:white; }
td.amount span { color:grey; }
</style>
<table width="100%" cellspacing="0" cellpadding="0">)";

    for (auto it = trades.begin(); count != 0 && it != trades.end(); ++it, --count)
    {
        // use toString and truncate to avoid tmp double?
        QString price = QString::number(it->price.getAsDouble(), 'f', priceDecs);
        QString amount = QString::number(it->size.getAsDouble(), 'f', amountDecs);
        QString time = it->time.toLocalTime().time().toString();

        // MakerBuy == TakerSell = downtick
        // MakerSell == TakerBuy = uptick
        switch (it->side)
        {
        case MakerSide::Buy:
            stream << "<tr><td class=\"down\">";
            break;
        case MakerSide::Sell:
            stream << "<tr><td class=\"up\">"; // -> down
            break;
        case MakerSide::None:
            stream << "<tr><td>";
            continue;
        }

        stream << Utils::DiffText(prevPrice, price) << "</td>"
               << "<td class=\"amount\">" << Utils::DiffText(prevAmount, amount) << "</td>"
               << "<td class=\"amount\">" << Utils::DiffText(prevTime, time) << "</td>"
               << "<\tr>";

        prevPrice = price;
        prevAmount = amount;
        prevTime = time;
    }

   stream << "</table>";
   tradesWidget.document()->setHtml(*stream.string());
}

void MainWindow::Candles(std::deque<Candle> values)
{
    this->ui->candleChart->SetCandles(std::move(values), granularity); // prevents elision?
}

void MainWindow::Trades(std::deque<Trade> values)
{
    this->trades = std::move(values); // prevents elision?
}

void MainWindow::Ticker(const Tick &tick)
{
//    if (!trades.empty())
//    {
        // tick sequenceNumber is batched as are the associated trades
        // for a complete trade list the HB channel could should fetch any missing trades, or do it here? or on a separate timer
        // so ticks should not be stored as a trade? unless identified as unbatched?
        //        if (ticks.front().sequence != tick.sequence -1)
        //        {
        //            log.Info(QString("Missed ticks %1 : %2").arg(ticks.front().sequence).arg(tick.sequence));
        //        }

        // currently ticks starts as a trade snaphot + aggregated trade tick updates
        // expect this code to onlt trigger at startup
//        Tick lastTick = ticks.front();
//        if (lastTick.sequence >= tick.sequence)
//        {
//            log.Info(QString("Tick already seen %1 : %2").arg(lastTick.sequence).arg(tick.sequence));
//        }
//        if (lastTick.tradeId >= tick.tradeId)
//        {
//            log.Info(QString("Trade already seen %1 : %2").arg(lastTick.tradeId).arg(tick.tradeId));
//        }
//    }
//    else
//    {
//        log.Info("1st tick, no trades yet");
//    }

    // masquerade aggregated tick as trade, todo fetch missing tradeId range, here or from HB?
    Trade trade;
    trade.time = tick.time;
    trade.tradeId = tick.tradeId;
    trade.price = tick.price;
    trade.size = tick.lastSize;
    trade.side = TakerToMaker(tick.side);

    trades.push_front(trade);
    if (trades.size()>100) // parm
    {
        trades.pop_back();
    }

    ui->candleChart->AddTick(tick);
}

void MainWindow::Heartbeat(const QDateTime & serverTime)
{
    ui->candleChart->Heartbeat(serverTime);
}

void MainWindow::StateChanged(GDL::ConnectedState state)
{
    switch (state)
    {
    case GDL::ConnectedState::NotConnected:
        ui->statusBar->setStyleSheet("background-color: red; color: white");
        ui->statusBar->showMessage("Not Connected");
        break;

    case GDL::ConnectedState::Connecting:
        ui->statusBar->setStyleSheet("color: yellow");
        ui->statusBar->showMessage("Connecting...");
        break;

    case GDL::ConnectedState::Connected:
        ui->statusBar->setStyleSheet("color: limegreen");
        ui->statusBar->showMessage("Connected");
        Connected();
        break;
    }
}

void MainWindow::GranularityChanged(QAction * action)
{
    granularity = (Granularity)action->data().toUInt();
    gdl->FetchAllCandles(granularity);
}

void MainWindow::Connected()
{
    // clear now to avoid ticks being added to old data will blank display while fetching
    // better model to store all data in an atomically swapable entity

    trades.clear();
    ui->candleChart->SetCandles({}, granularity);
    ui->orderBook->document()->clear();
    ui->trades->document()->clear();

    ui->depthChart->update();

    // calc value? want to be >= trade history window rows
    gdl->FetchTrades(100);
    gdl->FetchAllCandles(granularity);
}
