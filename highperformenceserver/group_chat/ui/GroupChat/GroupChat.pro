#-------------------------------------------------
#
# Project created by QtCreator 2016-04-11T14:49:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GroupChat
TEMPLATE = app


SOURCES += main.cpp\
        chatdialog.cpp \
    connectdialog.cpp \
    controller.cpp \
    client_poll.c

HEADERS  += chatdialog.h \
    connectdialog.h \
    controller.h \
    def.h

FORMS    += chatdialog.ui \
    connectdialog.ui

LIBS += -lpthread
