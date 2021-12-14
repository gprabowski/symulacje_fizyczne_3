QT += core gui widgets openglwidgets charts

CONFIG += c++17

SOURCES += \
    openglwidget.cpp \
    functions.cpp \
    object.cpp \
    simulationthread.cpp \
    main.cpp

HEADERS += \
    openglwidget.h \
    functions.h \
    object.h \
    mainwindow.h \
    simulationthread.h

FORMS += \
    mainwindow.ui \

RESOURCES  = shaders/shaders.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
