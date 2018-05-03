QT += websockets

INCLUDEPATH += \
    "..\..\..\github\decimal_for_cpp\include" \
    "..\..\..\github\rapidjson\include" \

debug
{
    LIBS += -L../GDaxLib/debug -lGDaxLib
}

release
{
    LIBS += -L../GDaxLib/release -lGDaxLib
}
