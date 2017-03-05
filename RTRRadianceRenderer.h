#ifndef __TINY_RENDERER_RTRRAIANCERENDERER_H__
#define __TINY_RENDERER_RTRRAIANCERENDERER_H__

#include <vector>
#include <QVector>
#include <Geometry/NearestSearchKdTree.h>
#include "libs/nanoflann/include/nanoflann.hpp"
#include "RTRVector3D.h"
#include "Photon.h"
#include "SamplingUtil.h"

class RTRModel;
class RTRKdTree;
class RTRCamera;
class QImage;
class RTRRenderElement;

struct NanoFlannPhotonAdaptor
{
    const std::vector<Photon*> &obj; //!< A const ref to the data set origin

                                    /// The constructor that sets the data set source
    NanoFlannPhotonAdaptor(const std::vector<Photon*> &obj_) : obj(obj_) { }

    /// CRTP helper method
    inline const std::vector<Photon*>& derived() const { return obj; }

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const { return derived().size(); }

    // Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
    inline double kdtree_distance(const double *p1, const size_t idx_p2, size_t /*size*/) const
    {
        const double d0 = p1[0] - derived()[idx_p2]->location(0);
        const double d1 = p1[1] - derived()[idx_p2]->location(1);
        const double d2 = p1[2] - derived()[idx_p2]->location(2);
        return d0*d0 + d1*d1 + d2*d2;
    }

    // Returns the dim'th component of the idx'th point in the class:
    // Since this is inlined and the "dim" argument is typically an immediate value, the
    //  "if/else's" are actually solved at compile time.
    inline double kdtree_get_pt(const size_t idx, int dim) const
    {
        return derived()[idx]->location(dim);
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }
};

typedef nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<double, NanoFlannPhotonAdaptor>,
    NanoFlannPhotonAdaptor, 3
> PhotonKdTree;

class AccessRTRVector3D
{
public:
    double operator()(Photon* photon, int i)
    {
        return photon->location(i);
    }
};

class RTRRadianceRenderer
{
public:
    RTRModel* model;
    QVector<RTRRenderElement*> elements;
    RTRKdTree* elementsCache;
    RTRCamera* camera;
    QImage* image;
    SamplingUtil* sampler;
    QVector<Photon*> allPhotons;
    QVector<Photon*> causticPhotons;
    std::vector<Photon*> stlCausticPhotons;
    std::vector<Photon*> stlDiffusePhotons;
    NanoFlannPhotonAdaptor *causticPhotonAdapter;
    NanoFlannPhotonAdaptor *diffusePhotonAdapter;
    PhotonKdTree *diffusePhotonMap = nullptr;
    PhotonKdTree *causticPhotonMap = nullptr;
    //NearestSearchKdTree<Photon*, double, 3, AccessRTRVector3D> *diffusePhotonMap = nullptr;
    //NearestSearchKdTree<Photon*, double, 3, AccessRTRVector3D> *causticPhotonMap = nullptr;
public:
    void renderPhoton(
        RTRVector3D location, RTRVector3D direction, QVector<Photon*> &result,
        RTRRenderElement *emissionElement, RTRColor lightColor, bool causticOnly);
    void execute();
};

#endif // __TINY_RENDERER_RTRRAIANCERENDERER_H__
