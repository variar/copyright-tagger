#-------------------------------------------------
#
# Project created by QtCreator 2016-03-11T01:29:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = copyright_tagger
TEMPLATE = app

LIBS += -lexiv2

win32{
DEFINES += EXV_UNICODE_PATH
}

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    droparea.cpp

HEADERS  += mainwindow.hpp \
    droparea.hpp

FORMS    += mainwindow.ui

RESOURCES += \
    main.qrc

RC_FILE = main.rc
