#include "mainwindow.h"

#include "depthchart.h"
#include "utils.h"

#include <QTextEdit>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
    , timer(std::make_unique<QTimer>())
{
    ui->setupUi(this);

    ui->depthChart->SetGDaxLib(&gDaxLib);

    connect(timer.get(), &QTimer::timeout, this, &MainWindow::Update);
    connect(&restProvider, &RestProvider::OnCandles, this, &MainWindow::Candles);
    connect(&restProvider, &RestProvider::OnTrades, this, &MainWindow::Trades);
    connect(&gDaxLib, &GDaxLib::OnTick, this, &MainWindow::Ticker);
    connect(&gDaxLib, &GDaxLib::OnStateChanged, this, &MainWindow::StateChanged);

    timer->start(5000);
}

void MainWindow::Update()
{
    ui->depthChart->update();
    ui->candleChart->update();
    GenerateOrderBook();
    GenerateTradeList();

    gDaxLib.Ping();
}

void MainWindow::on_actionE_xit_triggered()
{
    QApplication::quit();
}

void MainWindow::GenerateOrderBook()
{
    auto & orderBook = *ui->orderBook;
    QFont font = orderBook.document()->defaultFont();
    QFontMetrics fm(font);
    int fontHeight = fm.height();
    int lines = orderBook.height()/2/fontHeight-1;

    const auto & asks = gDaxLib.Asks();
    const auto & bids = gDaxLib.Bids();
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
                   .arg(DiffText(prevPrice, price))
                   .arg(DiffText(prevAmount, amount))
                   .arg(DiffText(prevTotAmount, totAmount));

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
               << "<td class=\"up\">" << DiffText(prevPrice, price) << "</td>"
               << "<td class=\"amount\">" << DiffText(prevAmount, amount) << "</td>"
               << "<td class=\"amount\">" << DiffText(prevTotAmount, totAmount) << "</td>"
               << "<\tr>";
        prevPrice = price;
        prevAmount = amount;
        prevTotAmount = totAmount;
   }

   stream << "</table>";
   orderBook.document()->setHtml(*stream.string());
}

void MainWindow::GenerateTradeList()
{
    auto & tradesWidget = *ui->trades;
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

        stream << DiffText(prevPrice, price) << "</td>"
               << "<td class=\"amount\">" << DiffText(prevAmount, amount) << "</td>"
               << "<td class=\"amount\">" << DiffText(prevTime, time) << "</td>"
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

void MainWindow::Ticker(Tick tick)
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
}

void MainWindow::StateChanged(GDaxLib::State state)
{
    switch (state)
    {
    case GDaxLib::State::NotConnected:
        ui->statusBar->setStyleSheet("background-color: red; color: white");
        ui->statusBar->showMessage("Not Connected");
        break;

    case GDaxLib::State::Connecting:
        ui->statusBar->setStyleSheet("color: yellow");
        ui->statusBar->showMessage("Connecting...");
        break;

    case GDaxLib::State::Connected:
        ui->statusBar->setStyleSheet("color: limegreen");
        ui->statusBar->showMessage("Connected");
        Connected();
        break;
    }
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
    restProvider.FetchTrades(100);

    restProvider.FetchAllCandles(granularity);
}
