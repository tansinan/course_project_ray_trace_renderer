#-------------------------------------------------
#
# Project created by QtCreator 2015-04-22T12:25:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RayTraceRenderer
TEMPLATE = app


SOURCES += main.cpp\
    RTRModel.cpp \
    MainWindow.cpp \
    RTRViewer.cpp \
    RTRMaterial.cpp \
    RTRCamera.cpp \
    RTRMatrix.cpp \
    RTRRenderer.cpp \
    RTRGeometry.cpp \
    Light/RTRLightPoint.cpp \
    RTRRenderElement.cpp \
    RTRKdTree.cpp

HEADERS  += \
    RTRModel.h \
    MainWindow.h \
    RTRViewer.h \
    RTRMaterial.h \
    RTRCamera.h \
    RTRMatrix.h \
    RTRRenderer.h \
    RTRGeometry.h \
    Light/RTRLightAbstract.h \
    Light/RTRLightPoint.h \
    RTRRenderElement.h \
    RTRKdTree.h

FORMS    +=
