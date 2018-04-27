TEMPLATE = subdirs

SUBDIRS += \
    GDaxLib \
    GDaxConsole \
    GDaxConsole2 \
    GDaxGui

CONFIG += ordered

GDaxLib.subdir = GDaxLib
GDaxConsole.subdir = GDaxConsole
GDaxConsole2.subdir = GDaxConsole2
GDaxGui.subdir = GDaxGui

GDaxConsole.depends = GDaxLib
GDaxConsole2.depends = GDaxLib
GDaxGui.depends = GDaxLib
