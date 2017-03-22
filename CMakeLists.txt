cmake_minimum_required(VERSION 3.0)
project(TinyRenderer)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

find_package(Embree 2 REQUIRED)
include_directories(${EMBREE_INCLUDE_PATH})

include_directories(
  ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS})

add_definitions(${Qt5Widgets_DEFINITIONS})

find_package(Qt5Widgets REQUIRED)

#add_library(DeformDialog.ui DeformDialog.ui)

add_executable(${PROJECT_NAME}
    MainWindow.cpp
    RTRCamera.cpp
    RTRColor.cpp
    RTRGeometry.cpp
    Light/RTRLightPoint.cpp
    RTRMaterial.cpp
    RTRModel.cpp
    RTRRenderElement.cpp
    RTRRenderer.cpp
    RTRVector2D.cpp
    RTRVector3D.cpp
    RTRViewer.cpp
    main.cpp
    RTRRenderThread.cpp
    PhotonMapGenerationProcedure.cpp
    RTRRadianceRenderer.cpp
    SamplingUtil.cpp
    RTRMatrix.cpp
    RenderingOptions.cpp
    RayTracing/AbstractRayTracingKernel.cpp
    RayTracing/SimpleKdTreeRayTracingKernel.cpp
    RayTracing/EmbreeRayTracingKernel.cpp
)
qt5_use_modules(${PROJECT_NAME} Widgets Gui Core)
target_link_libraries(${PROJECT_NAME} embree)
