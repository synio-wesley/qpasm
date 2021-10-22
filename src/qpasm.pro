# -------------------------------------------------
# Project created by QtCreator 2009-04-16T12:19:54
# -------------------------------------------------
QT += core gui svg
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = qpasm
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    highlighter.cpp \
    libpasm/helper.c \
    libpasm/interpreter.c \
    libpasm/memory.c \
    libpasm/ui.c \
    aboutdialog.cpp \
    qtsignalbridge.cpp \
    codeedit.cpp \
    pasmthread.cpp \
    libpasm/debugger.c \
    listwidgetitem.cpp \
    addmemdialog.cpp \
    linevalidator.cpp \
    libpasm/errorcodes.c \
    configdialog.cpp
HEADERS += mainwindow.h \
    highlighter.h \
    libpasm/pasm.h \
    libpasm/errorcodes.h \
    libpasm/opcodes.h \
    libpasm/helper.h \
    libpasm/interpreter.h \
    libpasm/memory.h \
    libpasm/ui.h \
    aboutdialog.h \
    qtsignalbridge.h \
    codeedit.h \
    pasmthread.h \
    libpasm/debugger.h \
    listwidgetitem.h \
    addmemdialog.h \
    linevalidator.h \
    configdialog.h
FORMS += mainwindow.ui \
    aboutdialog.ui \
    addmemdialog.ui \
    configdialog.ui
RESOURCES += qpasm.qrc
TRANSLATIONS += translations/qpasm_nl.ts
CONFIG += release
