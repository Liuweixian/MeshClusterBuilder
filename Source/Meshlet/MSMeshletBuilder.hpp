//
//  MSMeshletBuilder.hpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#ifndef MSMeshletBuilder_hpp
#define MSMeshletBuilder_hpp

#include <stdio.h>
#include <vector>
#include "MeshClusterBuilder.hpp"
#include "Vector4.h"

template <typename T>
struct InlineMeshlet
{
    struct PackedTriangle
    {
        uint32_t i0 : 10;
        uint32_t i1 : 10;
        uint32_t i2 : 10;
        uint32_t spare : 2;
    };

    std::vector<T>              UniqueVertexIndices;
    std::vector<PackedTriangle> PrimitiveIndices;
};

class MSMeshletBuilder : public MeshClusterBuilder
{
public:
    MSMeshletBuilder();
    ~MSMeshletBuilder();
    template<class IndexType>
    void Build(const Vector3f* pVertexData, const UInt32 nVertexDataCount, const IndexType* pIndexData, const UInt32 nIndexDataCount, const AABB bounds, int& nClusterCount, MeshCluster** pMeshCluster);
private:
    Vector3f ComputeNormal(Vector3f* tri);
    template <typename T>
    float ComputeScore(const InlineMeshlet<T>& meshlet, Vector4f sphere, Vector4f normal, T (&triIndices)[3], Vector3f* triVerts);

    template <typename T>
    bool AddToMeshlet(uint32_t maxVerts, uint32_t maxPrims, InlineMeshlet<T>& meshlet, T (&tri)[3]);

    template <typename T>
    bool IsMeshletFull(uint32_t maxVerts, uint32_t maxPrims, const InlineMeshlet<T>& meshlet);
};
#endif /* MSMeshletBuilder_hpp */
