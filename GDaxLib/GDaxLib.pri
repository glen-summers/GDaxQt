QT += websockets

INCLUDEPATH += \
    "../GDaxLib" \
    "../../../github/decimal_for_cpp/include" \
    "../../../github/rapidjson/include" \

debug {
    LIBS += -L../GDaxLib/debug -lGDaxLib
}

release {
    LIBS += -L../GDaxLib/release -lGDaxLib
}

win32 {
    CONFIG(debug, debug|release) {
        PRE_TARGETDEPS += ../GDaxLib/debug/GDaxLib.lib
    } else {
        PRE_TARGETDEPS += ../GDaxLib/release/GDaxLib.lib
    }
}
