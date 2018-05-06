#include "mainwindow.h"

#include "graphicswidget.h"

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

    auto textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    QTextDocument *document = textEdit->document();
    document->setHtml(R"(
<style>
td { color:red; }
span { color:darkred; }
</style>
<table>
<tr><td>red<span>red</span></td></tr>
<tr><td><span>red</span>red</td></tr>
</table>)");

    ui->horizontalLayout->addWidget(textEdit);

    //
    connect(timer.get(), SIGNAL(timeout()), this, SLOT(onUpdate()));
    timer->start(1000);
}

void MainWindow::onUpdate()
{
    graphicsWidget.update();
}

void MainWindow::on_actionE_xit_triggered()
{
    QApplication::quit();
}
