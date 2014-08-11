TEMPLATE = lib

TARGET = CQXml

DEPENDPATH += .

QT += widgets

CONFIG += staticlib

QMAKE_CXXFLAGS += -std=c++11

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
../../CXML/include \
../../CStrUtil/include \
../../CUtil/include \
