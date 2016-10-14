QT += core network
QT -= gui

CONFIG += c++11

TARGET = exampleTcpServer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    main_server.cpp \
    server_setup.cpp \
    connection_thread.cpp

HEADERS += \
    application_ports.h \
    main_server.h \
    server_setup.h \
    connection_thread.h
