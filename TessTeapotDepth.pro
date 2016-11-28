QT += gui core

CONFIG += c++11

TARGET = TessTeapotDepth
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    TessTeapotDepth.cpp \
    vboteapotpatch.cpp

HEADERS += \
    TessTeapotDepth.h \
    vboteapotpatch.h

OTHER_FILES += \
    vshader.txt \
    fshader.txt \
    tcsshader.txt \
    tesshader.txt \
    gshader.txt

RESOURCES += \
    shaders.qrc

DISTFILES += \
    fshader.txt \
    vshader.txt \
    tcsshader.txt \
    tesshader.txt \
    gshader.txt
