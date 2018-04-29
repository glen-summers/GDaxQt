#include(C:\Users\Glen\source\repos\GDaxQt\GDaxLib\GDaxLib.pri)
#!include(C:\Users\Glen\source\repos\GDaxQt\GDaxLib\GDaxLib.pri)
#{
#    error( "Lib not found" )
#}

#error($$(PWD))

QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp

