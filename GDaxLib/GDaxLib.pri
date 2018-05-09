!include("misc.pri") {
    warning("misc.pri not found")
}

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
        PRE_TARGETDEPS += $$OUT_PWD/../GDaxLib/debug/GDaxLib.lib
    } else {
        PRE_TARGETDEPS += $$OUT_PWD/../GDaxLib/release/GDaxLib.lib
    }
}

unix {
    PRE_TARGETDEPS += $$OUT_PWD/../GDaxLib/libGDaxLib.a
}
