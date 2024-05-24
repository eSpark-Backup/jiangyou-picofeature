QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Set a define to tell the codebase that compilation is being done by Qt
DEFINES += QT_CREATOR

SOURCES += \
    actuator_lib.cpp \
    main.cpp \
    mainwindow.cpp 
    
SOURCES += $$files(serial/*.cc)
SOURCES += $$files(serial/impl/*.cc)
SOURCES += $$files(serial/impl/list_ports/*.cc)

HEADERS += \
    actuator_lib.h \
    mainwindow.h \

HEADERS += $$files(serial/*.h)
HEADERS += $$files(serial/impl/*.h)

FORMS += \
    mainwindow.ui

# Add the setupapi library for windows
win32: LIBS += -lsetupapi

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
