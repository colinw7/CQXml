TEMPLATE = lib

TARGET = CQXml

DEPENDPATH += .

QT += widgets

CONFIG += staticlib

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
../../CUtil/include \
