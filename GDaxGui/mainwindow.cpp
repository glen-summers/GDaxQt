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

    ui->depthChart->setGDaxLib(&g);

    connect(timer.get(), SIGNAL(timeout()), this, SLOT(onUpdate()));
    connect(&restProvider, SIGNAL(candles(std::vector<Candle>)), this, SLOT(setCandles(std::vector<Candle>)));
    connect(&restProvider, SIGNAL(trades(std::vector<Trade>)), this, SLOT(setTrades(std::vector<Trade>)));
    connect(&g, &GDaxLib::tick, this, &MainWindow::onTick);

    timer->start(1000);

    restProvider.fetchTrades();
    restProvider.fetchCandles();
}

void MainWindow::onUpdate()
{
    ui->depthChart->update();
    ui->candleChart->update();
    generateOrderBook();
    generateTradeList();
}

void MainWindow::on_actionE_xit_triggered()
{
    QApplication::quit();
}

void MainWindow::generateOrderBook()
{
    auto & orderBook = *ui->orderBook;
    QFont font = orderBook.document()->defaultFont();
    QFontMetrics fm(font);
    int fontHeight = fm.height();
    int lines = orderBook.height()/2/fontHeight-1;

    const auto & asks = g.Asks();
    const auto & bids = g.Bids();
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
td.ask { color:red; }
td.ask span { color:darkred; }
td.bid { color:limegreen; }
td.bid span { color:darkgreen; }
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

        reverso << QString(R"(<tr><td class="ask">%1</td><td class="amount">%2</td><td class="amount">%3</td><\tr>)")
                   .arg(diffText(prevPrice, price))
                   .arg(diffText(prevAmount, amount))
                   .arg(diffText(prevTotAmount, totAmount));

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
               << "<td class=\"bid\">" << diffText(prevPrice, price) << "</td>"
               << "<td class=\"amount\">" << diffText(prevAmount, amount) << "</td>"
               << "<td class=\"amount\">" << diffText(prevTotAmount, totAmount) << "</td>"
               << "<\tr>";
        prevPrice = price;
        prevAmount = amount;
        prevTotAmount = totAmount;
   }

   stream << "</table>";
   orderBook.document()->setHtml(*stream.string());
}

void MainWindow::generateTradeList()
{
    auto & trades = *ui->trades;
    QFont font = trades.document()->defaultFont();
    QFontMetrics fm(font);
    int fontHeight = fm.height();
    int count = trades.height()/fontHeight;

    int priceDecs = 2;
    int amountDecs = 4;

    // just use std::ostringstream?
    QString prevPrice, prevAmount, prevTime;
    QString html;
    QTextStream stream(&html); // perf test?

    // subset of html - http://doc.qt.io/qt-5/richtext-html-subset.html
    stream << R"(<style>
td { text-align:right; }
td.ask { color:red; }
td.ask span { color:darkred; }
td.bid { color:limegreen; }
td.bid span { color:darkgreen; }
td.mid { color:white; padding: 10px; }
td.amount { color:white; }
td.amount span { color:grey; }
</style>
<table width="100%" cellspacing="0" cellpadding="0">)";

    for (auto it = ticks.begin(); count != 0 && it != ticks.end(); ++it, --count)
    {
        // use toString and truncate to avoid tmp double?
        QString price = QString::number(it->price.getAsDouble(), 'f', priceDecs);
        QString amount = QString::number(it->lastSize.getAsDouble(), 'f', amountDecs);
        QString time = it->time.toLocalTime().time().toString();

        // MakerBuy == TakerSell = downtick
        // MakerSell == TakerBuy = uptick
        // bid|ask css just use up\down?
        switch (it->side)
        {
        case TakerSide::Buy:
            stream << "<tr><td class=\"bid\">"; // -> up
            break;
        case TakerSide::Sell:
            stream << "<tr><td class=\"ask\">"; // -> down
            break;
        case TakerSide::None:
            stream << "<tr><td>";
            continue;
        }

        stream << diffText(prevPrice, price) << "</td>"
               << "<td class=\"amount\">" << diffText(prevAmount, amount) << "</td>"
               << "<td class=\"amount\">" << diffText(prevTime, time) << "</td>"
               << "<\tr>";

        prevPrice = price;
        prevAmount = amount;
        prevTime = time;
    }

   stream << "</table>";
   trades.document()->setHtml(*stream.string());
}

void MainWindow::setCandles(std::vector<Candle> candles)
{
    this->ui->candleChart->setCandles(std::move(candles));
}

void MainWindow::setTrades(std::vector<Trade> trades)
{
    // need to merge into existing ticks
    for (auto t : trades)
    {
        Tick tick{};
        tick.tradeId = t.tradeId;
        tick.time = t.time;
        tick.price = t.price;
        tick.side= ToTaker(t.side);
        tick.lastSize = t.size;
        ticks.push_back(tick);
    }
}

void MainWindow::onTick(Tick tick)
{
    if (!ticks.empty())
    {
        if (ticks.front().sequence != tick.sequence -1)
        {
            log.Info(QString("Missed ticks %1 : %2").arg(ticks.front().sequence).arg(tick.sequence));
        }
    }

    ticks.push_front(tick);
    if (ticks.size()>100) // parm
    {
        ticks.pop_back();
    }

}
