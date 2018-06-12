!include("../GDaxLib/GDaxLib.pri") {
    warning("GDaxLib.pri not found")
}

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GDaxGui
TEMPLATE = app
win32:RC_ICONS += main.ico

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    candlechart.cpp \
    depthchart.cpp \
    plot.cpp \
    candleoverlay.cpp \
    expandbutton.cpp \
    touchhandler.cpp

HEADERS += \
    mainwindow.h \
    candlechart.h \
    depthchart.h \
    plot.h \
    sma.h \
    ema.h \
    overlaywidget.h \
    candleoverlay.h \
    expandbutton.h \
    touchhandler.h

FORMS += \
        mainwindow.ui
