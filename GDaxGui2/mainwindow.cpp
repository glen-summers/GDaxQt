#include "mainwindow.h"

#include "graphicswidget.h"
#include "utils.h"

#include <QTextEdit>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
    , timer(std::make_unique<QTimer>())
    , graphicsWidget(*new GraphicsWidget(g))
{
    ui->setupUi(this);
    ui->horizontalLayout->addWidget(&graphicsWidget);

    orderBook.setReadOnly(true);
    ui->horizontalLayout->addWidget(&orderBook);

    connect(timer.get(), SIGNAL(timeout()), this, SLOT(onUpdate()));
    timer->start(1000);
}

void MainWindow::onUpdate()
{
    graphicsWidget.update();
    generateOrderBook();
}

void MainWindow::on_actionE_xit_triggered()
{
    QApplication::quit();
}

void MainWindow::generateOrderBook()
{
    // calc number of lines from height
    //const auto & bids = g.Bids();
    const auto & asks = g.Asks();
    int lines = 10;
    int priceDecs = 2;
    int amountDecs = 4;
    Decimal tot;

    int count = lines;

    // hilites should be left\right split
    // use divs rather than tables? weak? html5

    // just use std::ostringstream?
    QString prevPrice, prevAmount, prevTotAmount;
    QString html;
    QTextStream stream(&html); // perf test?
    stream << R"(<style>
td, th { padding: 2px; }
td { color:darkred; }
span { color:red; }
</style>
<table>
)";

    // reverse!
    for (auto it = asks.begin(); count != 0 && it != asks.end(); ++it, --count)
    {
        auto p = it->first;
        auto a = it->second;
        tot += a;
        // use toString and truncate to avoid tmp double?
        QString price = QString::number(p.getAsDouble(), 'f', priceDecs);
        QString amount = QString::number(a.getAsDouble(), 'f', amountDecs);
        QString totAmount = QString::number(tot.getAsDouble(), 'f', amountDecs);

        stream << "<tr>"
               << "<td>" << diffText(prevPrice, price) << "</td>"
               << "<td>" << diffText(prevAmount, amount) << "</td>"
               << "<td>" << diffText(prevTotAmount, totAmount) << "</td>"
               << "<\tr>";
        prevPrice = price;
        prevAmount = amount;
        prevTotAmount = totAmount;
   }
   stream << "</table>";
   orderBook.document()->setHtml(*stream.string());

//orderBook.document()->setHtml(R"(
//<style>
//td { color:red; }
//span { color:darkred; }
//</style>
//<table>
//<tr><td>red<span>red</span></td></tr>
//<tr><td><span>red</span>red</td></tr>
//</table>)");
}
