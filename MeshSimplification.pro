#-------------------------------------------------
#
# Project created by QtCreator 2015-03-14T17:46:04
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MeshSimplification
TEMPLATE = app

CONFIG += c++11


SOURCES += main.cpp\
        mainwindow.cpp \
    meshviewer.cpp \
    mesh.cpp

HEADERS  += mainwindow.h \
    meshviewer.h \
    mesh.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L"C:/Program Files (x86)/OpenMesh 3.3/lib/" -lOpenMeshCore -lOpenMeshTools
else:win32:CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/OpenMesh 3.3/lib/" -lOpenMeshCored -lOpenMeshToolsd

INCLUDEPATH += "C:/Program Files (x86)/OpenMesh 3.3/include"
DEPENDPATH += "C:/Program Files (x86)/OpenMesh 3.3/include"

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += "C:/Program Files (x86)/OpenMesh 3.3/lib/libOpenMeshCore.a"
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += "C:/Program Files (x86)/OpenMesh 3.3/lib/libOpenMeshCored.a"
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += "C:/Program Files (x86)/OpenMesh 3.3/lib/OpenMeshCore.lib"
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += "C:/Program Files (x86)/OpenMesh 3.3/lib/OpenMeshCored.lib"
