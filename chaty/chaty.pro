#-------------------------------------------------
#
# Project created by QtCreator 2016-02-14T18:47:56
#
#-------------------------------------------------
lessThan(QT_MAJOR_VERSION , 5){
    lessThan(QT_MINOR_VERSION, 6){
        error("Application requires Qt 5.6")
    }
}

greaterThan(QT_MAJOR_VERSION , 4){
    greaterThan(QT_MINOR_VERSION, 6){
        message("Application is optimized for 5.6")
    }
}

QT += core gui widgets sql multimedia qml quick quickwidgets

TARGET = chaty

TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    aes_wrapper.cpp \
    ../shared/rsa_wrapper.cpp \
    ../shared/jsoncpp.cpp \
    ../shared/response_json_wrapper.cpp \
    ../shared/request_json_wrapper.cpp \
    ../shared/aes_json_wrapper.cpp \
    ../shared/mime_json_wrapper.cpp \
    ../shared/crypto_helper.cpp \
    ../shared/login_message_json_wrapper.cpp \
    ../shared/contact_in_export_json_wrapper.cpp \
    ../shared/ip_refresh_json_wrapper.cpp \
    ../shared/multi_lines_response_json_wrapper.cpp \
    ../shared/tcp_client.cpp \
    ../shared/file_io_helper.cpp \
    ../shared/my_rsa_keys_handler.cpp \
    local_storage.cpp \
    chat_style_helper.cpp \
    current_configuration.cpp \  
    mainwindow_controller.cpp \
    client_server_setup.cpp \
    client_server.cpp \
    client_connection_thread.cpp \
    background_transmitter.cpp \
    qrencode/bitstream.c \
    qrencode/mask.c \
    qrencode/mmask.c \
    qrencode/mqrspec.c \
    qrencode/qrencode.c \
    qrencode/qrinput.c \
    qrencode/qrspec.c \
    qrencode/rscode.c \
    qrencode/split.c \
    qr_dialog.cpp \
    imageprocessor.cpp \
    login_dialog.cpp \
    background_scheduler.cpp \
    messagebox_yes_no_dialog.cpp \
    edit_contacts_dialog.cpp \
    contact.cpp \
    ../shared/rsakeys_im_export_json_wrapper.cpp

HEADERS  += mainwindow.h \
    aes_wrapper.h \
    ../shared/rsa_wrapper.h \
    ../shared/response_json_wrapper.h \
    ../shared/aes_json_wrapper.h \
    ../shared/request_json_wrapper.h \
    ../shared/transmissiontype.h \
    ../shared/status_codes.h \
    ../shared/mime_json_wrapper.h \
    ../shared/crypto_helper.h \
    ../shared/ip_refresh_json_wrapper.h \
    ../shared/multi_lines_response_json_wrapper.h \
    ../shared/login_message_json_wrapper.h \
    ../shared/contact_in_export_json_wrapper.h \
    ../shared/application_ports.h \
    ../shared/tcp_client.h \
    ../shared/file_io_helper.h \
    ../shared/my_rsa_keys_handler.h \
    local_storage.h \
    chat_style_helper.h \
    current_configuration.h \   
    mainwindow_controller.h \  
    client_server_setup.h \
    client_server.h \
    client_connection_thread.h \
    background_transmitter.h \
    qrencode/bitstream.h \
    qrencode/mask.h \
    qrencode/mmask.h \
    qrencode/mqrspec.h \
    qrencode/qrencode.h \
    qrencode/qrencode_inner.h \
    qrencode/qrinput.h \
    qrencode/qrspec.h \
    qrencode/rscode.h \
    qrencode/split.h \
    qr_dialog.h \
    imageprocessor.h \
    login_dialog.h \
    background_scheduler.h \
    messagebox_yes_no_dialog.h \
    edit_contacts_dialog.h \
    contact.h \
    ../shared/rsakeys_im_export_json_wrapper.h

FORMS    += mainwindow.ui \
    qr_dialog.ui \
    login_dialog.ui \
    edit_contacts_dialog.ui

CONFIG += c++11

RESOURCES += \
    resources.qrc \
    camera/declarative-camera.qrc \

include(QZXing/QZXing.pri)
include(deployment.pri)

win32 {
    LIBS += -LC:/OpenSSL-Win32/lib/ -lssleay32 -llibeay32

    INCLUDEPATH += C:/OpenSSL-Win32/include
    DEPENDPATH += C:/OpenSSL-Win32/include
}

android {

    INCLUDEPATH += C:\Android\OpenSSL\openssl-1.0.2\include\

    LIBS += -LC:\Android\OpenSSL\openssl-1.0.2\armeabi-v7a\lib -lcrypto

    INCLUDEPATH += C:\Android\OpenSSL\openssl-1.0.2\armeabi-v7a
    DEPENDPATH += C:\Android\OpenSSL\openssl-1.0.2\armeabi-v7a

    PRE_TARGETDEPS += C:\Android\OpenSSL\openssl-1.0.2\armeabi-v7a\lib\libcrypto.a

    #Use of the AndroidManifest
    DISTFILES += \
        android/AndroidManifest.xml \
        android/gradle/wrapper/gradle-wrapper.jar \
        android/gradlew \
        android/res/values/libs.xml \
        android/build.gradle \
        android/gradle/wrapper/gradle-wrapper.properties \
        android/gradlew.bat

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}

linux {
    LIBS +=-lssl -lcrypto
    PKGCONFIG += openssl
    INCLUDEPATH += /usr/include/openssl

    target.path = $$[QT_INSTALL_EXAMPLES]/quick/quickwidgets/quickwidget
    INSTALLS += target
}

macx {
    LIBS += -lssl -lcrypto
    PKGCONFIG += openssl
    INCLUDEPATH += /usr/local/opt/openssl/include
}

ios {
    LIBS +=-L/usr/local/opt/openssl/lib -lssl -lcrypto
    PKGCONFIG += openssl
    INCLUDEPATH += /usr/local/opt/openssl/include
}
