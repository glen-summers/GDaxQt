CONFIG += c++17
win32: {
    QMAKE_CXXFLAGS_WARN_ON -= -W3
    QMAKE_CXXFLAGS_WARN_ON += -W4
    QMAKE_CXXFLAGS += /std:c++17 /permissive- /DNOMINMAX
}

unix: {
    QMAKE_CXXFLAGS += -std=c++17
}

#avoid here...
QT += websockets

INCLUDEPATH += \
    $$PWD/../GDaxLib \
    $$PWD/../Dependencies/decimal_for_cpp/include \
    $$PWD/../Dependencies/rapidjson/include \
