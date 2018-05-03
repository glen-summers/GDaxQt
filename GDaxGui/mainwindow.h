#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "../GDaxLib/gdaxlib.h"
#include <memory>

class MainWindow : public QWindow
{
    static constexpr Qt::GlobalColor BidFillColour = Qt::GlobalColor::darkGreen;
    static constexpr Qt::GlobalColor BidEdgeColour = Qt::GlobalColor::green;
    static constexpr Qt::GlobalColor AskFillColour = Qt::GlobalColor::darkRed;
    static constexpr Qt::GlobalColor AskEdgeColour = Qt::GlobalColor::red;

    Q_OBJECT

    GDaxLib g;
    std::unique_ptr<QBackingStore> backingStore;
    std::unique_ptr<QTimer> timer;

public:
    explicit MainWindow(QWindow *parent = nullptr);

public slots:
    void renderLater();
    void renderNow();
    void onUpdate();

protected:
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void exposeEvent(QExposeEvent *event) override;

private:
    void render(QPainter & painter);
};

#endif
