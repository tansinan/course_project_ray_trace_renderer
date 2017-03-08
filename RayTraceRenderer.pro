QT += core widgets gui
TARGET = RayTraceRenderer
TEMPLATE = app

HEADERS += ./RTRModelPolygen.h \
    ./RTRTexture.h \
    ./RTRVector2D.h \
    ./RTRVector3D.h \
    ./RTRCamera.h \
    ./RTRColor.h \
    ./RTRGeometry.h \
    ./RTRKdTree.h \
    ./Light/RTRLightAbstract.h \
    ./Light/RTRLightPoint.h \
    ./RTRMaterial.h \
    ./RTRMatrix.h \
    ./RTRModel.h \
    ./RTRModelObject.h \
    ./RTRModelVertex.h \
    ./RTRRenderElement.h \
    ./RTRRenderer.h \
    ./MainWindow.h \
    ./RTRViewer.h \
    ./RTRRenderThread.h \
    PhotonMapGenerationProcedure.h \
    Photon.h \
    RTRRadianceRenderer.h \
    SamplingUtil.h \
    Geometry/NearestSearchKdTree.h \
    libs/nanoflann/include/nanoflann.hpp \
    RenderingOptions.h
	
SOURCES += ./MainWindow.cpp \
    ./RTRCamera.cpp \
    ./RTRColor.cpp \
    ./RTRGeometry.cpp \
    ./RTRKdTree.cpp \
    ./Light/RTRLightPoint.cpp \
    ./RTRMaterial.cpp \
    ./RTRModel.cpp \
    ./RTRRenderElement.cpp \
    ./RTRRenderer.cpp \
    ./RTRVector2D.cpp \
    ./RTRVector3D.cpp \
    ./RTRViewer.cpp \
    ./main.cpp \
    ./RTRRenderThread.cpp \
    PhotonMapGenerationProcedure.cpp \
    RTRRadianceRenderer.cpp \
    SamplingUtil.cpp \
    RTRMatrix.cpp \
    RenderingOptions.cpp
