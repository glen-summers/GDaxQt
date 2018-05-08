!include("..\GDaxLib\GDaxLib.pri") {
    warning("GDaxLib.pri not found")
}

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GDaxGui
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    graphicswidget.cpp \
    candlechart.cpp

HEADERS += \
    mainwindow.h \
    graphicswidget.h \
    candlechart.h

FORMS += \
        mainwindow.ui
