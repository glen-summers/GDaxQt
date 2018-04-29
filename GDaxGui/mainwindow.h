#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "../GDaxLib/gdaxlib.h"
#include <memory>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    std::unique_ptr<Ui::MainWindow> ui;
    GDaxLib g;

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;
};

#endif
