TEMPLATE = app

TARGET = CQXmlTest

DEPENDPATH += .

QT += widgets

#CONFIG += debug

# Input
SOURCES += \
CQXmlTest.cpp \

HEADERS += \
CQXmlTest.h \

DESTDIR     = .
OBJECTS_DIR = .

INCLUDEPATH += \
../include \
../../CQStyleWidget/include \
.

unix:LIBS += \
-L../lib \
-L../../CQStyleWidget/lib \
-L../../CQUtil/lib \
-L../../CXML/lib \
-L../../CFile/lib \
-L../../COS/lib \
-L../../CStrUtil/lib \
-L../../CRegExp/lib \
-lCQXml -lCQStyleWidget -lCQUtil \
-lCXML -lCFile -lCOS -lCStrUtil -lCRegExp \
-ltre
