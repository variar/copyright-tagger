#-------------------------------------------------
#
# Project created by QtCreator 2016-03-11T01:29:01
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qexif
TEMPLATE = app

LIBS += -lexiv2

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    droparea.cpp

HEADERS  += mainwindow.hpp \
    droparea.hpp

FORMS    += mainwindow.ui

RESOURCES += \
    main.qrc
