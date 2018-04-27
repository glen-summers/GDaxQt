QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp

debug
{
    LIBS += -L../GDaxLib/debug -lGDaxLib
}

release
{
    LIBS += -L../GDaxLib/release -lGDaxLib
}
