QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Boss.cpp \
    Block.cpp \
    Effect.cpp \
    Enemy.cpp \
    FloatingPlatform.cpp \
    Gordo.cpp \
    HotHead.cpp \
    HUD.cpp \
    Kirby.cpp \
    Item.cpp \
    Sparky.cpp \
    StarBlock.cpp \
    StarBullet.cpp \
    Fireball.cpp \
    WaddleDee.cpp \
    WaddleDoo.cpp \
    main.cpp \
    mainwindow.cpp \
    Slope.cpp

HEADERS += \
    Boss.h \
    Block.h \
    Effect.h \
    Enemy.h \
    FloatingPlatform.h \
    Gordo.h \
    HotHead.h \
    HUD.h \
    Kirby.h \
    Item.h \
    Sparky.h \
    StarBullet.h \
    Fireball.h \
    Starblock.h \
    WaddleDee.h \
    WaddleDoo.h \
    mainwindow.h \
    Slope.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc \
    res.qrc
