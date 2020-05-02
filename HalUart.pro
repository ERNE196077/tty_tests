TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -pthread
LIBS += -pthread

SOURCES += \
        main.cpp \
        HalUartPosix.cpp

HEADERS += \
        BufList.hpp \
        HalUartPosix.hpp
