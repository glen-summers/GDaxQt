#-------------------------------------------------
#
# Project created by QtCreator 2018-04-27T13:34:58
#
#-------------------------------------------------

QT       -= gui

TARGET = GDaxLib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        gdaxlib.cpp

HEADERS += \
        gdaxlib.h

#unix {
#    target.path = /usr/lib
#    INSTALLS += target
#}

#INSTALLS += target

# need?
target.path = ../TargetPath
INSTALLS += target
