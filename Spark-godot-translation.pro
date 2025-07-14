#-------------------------------------------------
#
# Project created by QtCreator 2025-07-10T13:34:04
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 启用SSL支持
DESTDIR = $$absolute_path($$PWD/bin/)

TARGET = Spark-godot-translation
DESTDIR = $$PWD/bin/
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        appobject.cpp \
        main.cpp \
        mainwindow.cpp \
        translationworker.cpp

HEADERS += \
        appobject.h \
        mainwindow.h \
        translationworker.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qrc.qrc

# Windows下自动复制SSL库文件到bin目录
win32 {
    # 确保bin目录存在
    !exists($$DESTDIR) {
        system($$QMAKE_MKDIR $$shell_path($$DESTDIR))
    }
    
    # 使用QMAKE_POST_LINK在每次构建后自动复制SSL库文件
    QMAKE_POST_LINK += $$QMAKE_COPY $$shell_path($$PWD/ssl/libcrypto-1_1.dll) $$shell_path($$DESTDIR) $$escape_expand(\n\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$shell_path($$PWD/ssl/libssl-1_1.dll) $$shell_path($$DESTDIR)
    
    # 备选方案：使用INSTALLS机制（需要make install）
    # SSL_LIBS_SOURCE = $$PWD/ssl
    # SSL_LIBS_TARGET = $$DESTDIR
    # libcrypto.path = $$SSL_LIBS_TARGET
    # libcrypto.files = $$SSL_LIBS_SOURCE/libcrypto-1_1.dll
    # libssl.path = $$SSL_LIBS_TARGET
    # libssl.files = $$SSL_LIBS_SOURCE/libssl-1_1.dll
    # INSTALLS += libcrypto libssl

    RC_ICONS += icon_1.ico
}
