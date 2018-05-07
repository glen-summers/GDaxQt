#include "mainwindow.h"

#include "graphicswidget.h"
#include "utils.h"

#include <QTextEdit>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
    , timer(std::make_unique<QTimer>())
{
    ui->setupUi(this);

    ui->openGLWidget->setGDaxLib(&g);

    connect(timer.get(), SIGNAL(timeout()), this, SLOT(onUpdate()));
    timer->start(1000);
}

void MainWindow::onUpdate()
{
    ui->openGLWidget->update();
    generateOrderBook();
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

    // hilites should be left\right split

    // just use std::ostringstream?
    QString prevPrice, prevAmount, prevTotAmount;
    QString html;
    QTextStream stream(&html); // perf test?

    // subset of html - http://doc.qt.io/qt-5/richtext-html-subset.html
    stream << R"(<style>
td { text-align:right; }
td.ask { color:darkred; }
td.ask span { color:red; }
td.bid { color:darkgreen; }
td.bid span { color:limegreen; }
td.mid { color:white; padding: 10px; }
td.amount { color:grey; }
td.amount span { color:white; }
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

    // add mid\last trade
    stream << R"(<table width="100%" cellspacing="0" cellpadding="0">
<tr><td class="mid">Mid1234</td></tr>
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
               << "<td>" << diffText(prevTotAmount, totAmount) << "</td>"
               << "<\tr>";
        prevPrice = price;
        prevAmount = amount;
        prevTotAmount = totAmount;
   }

   stream << "</table>";
   orderBook.document()->setHtml(*stream.string());
}
