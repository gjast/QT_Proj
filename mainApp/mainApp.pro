QT += network core gui widgets printsupport
CONFIG += console
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    authdialog.cpp \
    infomainwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    tradingbotapiclient.cpp

HEADERS += \
    authdialog.h \
    infomainwindow.h \
    mainwindow.h \
    qcustomplot.h \
    tradingbotapiclient.h

FORMS += \
    authdialog.ui \
    infomainwindow.ui \
    mainwindow.ui \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
