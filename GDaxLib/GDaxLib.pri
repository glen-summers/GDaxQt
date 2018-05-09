CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17
QT += websockets

INCLUDEPATH += \
    $$PWD/../GDaxLib \
    $$PWD/../../../github/decimal_for_cpp/include \
    $$PWD/../../../github/rapidjson/include \

win32 {
    debug {
        LIBS += -L$$OUT_PWD/../GDaxLib/debug -lGDaxLib
    }

    release {
        LIBS += -L$$OUT_PWD/../GDaxLib/release -lGDaxLib
    }
}

unix {
    LIBS += -L$$OUT_PWD/../GDaxLib -lGDaxLib
}

win32 {
    CONFIG(debug, debug|release) {
        PRE_TARGETDEPS += ../GDaxLib/debug/GDaxLib.lib
    } else {
        PRE_TARGETDEPS += ../GDaxLib/release/GDaxLib.lib
    }
}

unix {
    PRE_TARGETDEPS += $$OUT_PWD/../GDaxLib/libGDaxLib.a
}
