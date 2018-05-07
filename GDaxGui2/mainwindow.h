#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gdaxlib.h"

#include "ui_mainwindow.h"

#include <QMainWindow>
#include <QTimer>
#include <QTextEdit>

#include <memory>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<QTimer> timer;

    GDaxLib g;

public:
    explicit MainWindow(QWidget *parent = 0);

public slots:
    void onUpdate();

private slots:
    void on_actionE_xit_triggered();

private:
    void generateOrderBook();
};

#endif // MAINWINDOW_H
