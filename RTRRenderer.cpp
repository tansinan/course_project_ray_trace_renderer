#include "RTRRenderer.h"
#include <QColor>
#include <QDebug>
#include <QImage>
#include <time.h>
#include "RTRGeometry.h"
#include "Light/RTRLightPoint.h"
#include "RTRRenderThread.h"
#include "RTRRadianceRenderer.h"
#include "RTRModelPolygen.h"

RTRRenderer::RTRRenderer(QImage *_image)
{
    model = NULL;
    camera = NULL;
    image = _image;
    renderResult = new RTRColor[image->width()*image->height()];
}

bool RTRRenderer::render(RTRModel* _model, RTRCamera* _camera, int pass)
{
    if (pass <= 0) return false;
    if (_model == NULL || _camera == NULL) return false;
    model = _model;
    camera = _camera;

    //Scan all polygens to be rendered.
    for (const RTRModelPolygen* face : model->polygens)
    {
        //if (face->objectName == "Plane") continue;
        //TODO: Convex only
        RTRVector3D point1, point2, point3;

        //将多边形的第一个顶点作为三角形的第一个顶m点

        point1 = face->vertices[0]->position;

        //而三角形的另外两个顶点从多边形剩下的顶点中取两两相邻的选取。

        for (int j = 1; j < face->vertices.size() - 1; j++)
        {
            //构建新的三角形


            point2 = face->vertices[j]->position;
            point3 = face->vertices[j + 1]->position;

            //TODO: 存在内存泄漏问题！

            RTRRenderElement* element = new RTRRenderElement(new RTRTriangle3D(point1, point2, point3), camera);
            element->objectName = face->objectName;
            element->material = model->materialLibrary[face->materialName];
            if (face->uvMaps.size() > 0)
            {
                element->vertexUVMaps[0] = face->uvMaps[0];
                element->vertexUVMaps[1] = face->uvMaps[j];
                element->vertexUVMaps[2] = face->uvMaps[j + 1];
            }
            element->useSmoothShading = face->smoothShading;
            if (face->smoothShading)
            {
                element->vertexNormals[0] = face->normals[0];
                element->vertexNormals[1] = face->normals[j];
                element->vertexNormals[2] = face->normals[j + 1];
            }
            elements.append(element);
        }
    }


    radianceRenderer = new RTRRadianceRenderer(this);
    for (auto element : elements)
    {
        if (element->material->emissionStrength > 0.0001)
        {
            //qDebug() << "Emission object!";
            emissionElements.push_back(element);
        }
    }

    //创建Kd树以便加速搜索。

    elementsCache = new SimpleKdTreeRayTracingKernel();
    elementsCache->buildIndex(elements);

    renderThreads = new RTRRenderThread*[8];
    for (int i = 0; i < 8; i++)
    {
        renderThreads[i] = new RTRRenderThread(this, i);
        renderThreads[i]->radianceRenderer = radianceRenderer;
        connect(renderThreads[i], SIGNAL(finished()), this, SLOT(onThreadFinished()));
        connect(renderThreads[i], SIGNAL(renderFinished(int)), this, SLOT(onRenderFinished(int)));
    }
    currentPass = 0;
    targetPass = pass;
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 12; j++)
        {
            renderGridPass[i][j] = 0;
        }
    }
    //Build the photon map
    radianceRenderer->model = model;
    radianceRenderer->elements = elements;
    radianceRenderer->elementsCache = elementsCache;
    radianceRenderer->sampler = &this->sampler;
    radianceRenderer->execute();

    auto& photons = radianceRenderer->allPhotons;

    qDebug() << photons.size();
    for(int i = 0; i < image->width(); i++)
    {
        for(int j = 0; j < image->height(); j++)
        {
            image->setPixel(i, j, RTRColor(0,0,0).toQtColor().rgb());
        }
    }
    int orr = 0;
    for(int i = 0; i < photons.size(); i++)
    {
        auto point = camera->transformPoint(photons[i]->location);
        //qDebug() << point.x() << point.y();
        //qDebug() << photons[i].color.r() << photons[i].color.g() << photons[i].color.b();
        if(point.x() < 0 || point.y() < 0 || point.x() > image->width() - 1 || point.y() > image->height() - 1)
        {
            orr++;
            continue;
        }
        renderPixel(point.x(), point.y(), 1.0, photons[i]->color);
    }
    qDebug() << "orr" << orr;
    allocateTask();
    emit renderStatusChanged();
    return true;
}

void RTRRenderer::renderPixel(int x, int y, double z, const RTRColor& color)
{
    //qDebug() << color.r();
    //qDebug() <<color.toQtColor();
    image->setPixel(x, image->height() - 1 - y, color.toQtColor().rgb());
}

void RTRRenderer::allocateTask(int threadId)
{
    if (threadId == -1)
    {
        for (int i = 0; i < 8; i++)
        {
            allocateTask(i);
        }
        return;
    }
    if (renderThreads[threadId]->isRunning()) return;
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 12; j++)
        {
            if (renderGridPass[i][j] < currentPass)
            {
                renderGridPass[i][j]++;
                renderThreads[threadId]->start(40 * i, 40 * i + 39, 40 * j, 40 * j + 39);
                return;
            }
        }
    }
    //currentPass++;
    //TODO: this means render finished.
    if (currentPass >= targetPass)
    {
        for (int i = 0; i < 8; i++)
        {
            if (!renderThreads[i]->isFinished()) return;
        }
        emit renderFinished();
        return;
    }
    currentPass++;
    renderGridPass[0][0]++;
    renderThreads[threadId]->start(0, 39, 0, 39);
}

void RTRRenderer::onRenderFinished(int threadId)
{
    //RTRRenderThread* thread = renderThreads[threadId];
}

void RTRRenderer::onThreadFinished()
{
    RTRRenderThread* thread = (RTRRenderThread*)QObject::sender();
    for (int i = thread->xBegin; i <= thread->xEnd; i++)
    {
        for (int j = thread->yBegin; j <= thread->yEnd; j++)
        {
            renderResult[i*image->height() + j] = renderResult[i*image->height() + j] + thread->renderResult[i*image->height() + j];
        }
    }
    allocateTask(thread->threadIndex);
    emit renderStatusChanged();
}
