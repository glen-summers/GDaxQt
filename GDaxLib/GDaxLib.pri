QT += websockets

debug
{
    LIBS += -L../GDaxLib/debug -lGDaxLib
}

release
{
    LIBS += -L../GDaxLib/release -lGDaxLib
}
