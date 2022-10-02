TEMPLATE = lib

TARGET = CQXml

DEPENDPATH += .

QT += widgets printsupport webkitwidgets

CONFIG += staticlib

QMAKE_CXXFLAGS += -std=c++17

# Input
HEADERS += \
../include/CQXml.h \

SOURCES += \
CQXml.cpp \

OBJECTS_DIR = ../obj

DESTDIR = ../lib

INCLUDEPATH += \
. \
../include \
../../CQStyleWidget/include \
../../CQPropertyTree/include \
../../CXML/include \
../../CStrUtil/include \
../../CUtil/include \
