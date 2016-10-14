QT += core network
QT -= gui

CONFIG += c++11

TARGET = exampleTcpClient
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    tcp_client.cpp

HEADERS += \
    tcp_client.h \
    application_ports.h
