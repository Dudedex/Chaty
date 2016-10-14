QT += core
QT += network widgets
QT -= gui
QT += sql

CONFIG += c++11

TARGET = chatyServer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    server_setup.cpp \
    main_server.cpp \
    ../shared/rsa_wrapper.cpp \
    ../shared/jsoncpp.cpp \
    ../shared/request_json_wrapper.cpp \
    ../shared/response_json_wrapper.cpp \
    ../shared/mime_json_wrapper.cpp \
    ../shared/ip_refresh_json_wrapper.cpp \
    ../shared/multi_lines_response_json_wrapper.cpp \
    ../shared/login_message_json_wrapper.cpp\
    ../shared/crypto_helper.cpp \
    ../shared/file_io_helper.cpp \
    ../shared/my_rsa_keys_handler.cpp \
    ../shared/rsakeys_im_export_json_wrapper.cpp \
    connection_thread.cpp \
    mysql_wrapper.cpp \
    ../shared/tcp_client.cpp

HEADERS += \
    server_setup.h \
    main_server.h \
    ../shared/rsa_wrapper.h \
    ../shared/transmissiontype.h \
    ../shared/request_json_wrapper.h \
    ../shared/response_json_wrapper.h \
    ../shared/mime_json_wrapper.h \
    ../shared/login_message_json_wrapper.h\
    ../shared/ip_refresh_json_wrapper.h \
    ../shared/multi_lines_response_json_wrapper.h \
    ../shared/crypto_helper.h \
    ../shared/application_ports.h \
    ../shared/file_io_helper.h \
    ../shared/status_codes.h \
    ../shared/my_rsa_keys_handler.h \
    ../shared/rsakeys_im_export_json_wrapper.h \
    connection_thread.h \
    mysql_wrapper.h \
    ../shared/tcp_client.h


win32:{
    LIBS += -LC:/OpenSSL-Win32/lib/ -lssleay32 -llibeay32

    INCLUDEPATH += C:/OpenSSL-Win32/include
    DEPENDPATH += C:/OpenSSL-Win32/include
}

linux: {
    LIBS +=-lssl -lcrypto
    PKGCONFIG += openssl
    INCLUDEPATH += /usr/include/openssl
}
