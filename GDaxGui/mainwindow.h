#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "../GDaxLib/gdaxlib.h"
#include <memory>

class MainWindow : public QWindow
{
    static constexpr QRgb BidFillColour = qRgb(0,100,0);
    static constexpr QRgb BidEdgeColour = qRgb(0,255,0);
    static constexpr QRgb AskFillColour = qRgb(139,0,0);
    static constexpr QRgb AskEdgeColour = qRgb(255,0,0);

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
