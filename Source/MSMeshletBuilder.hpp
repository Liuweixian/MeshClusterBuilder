//
//  MSMeshletBuilder.hpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#ifndef MSMeshletBuilder_hpp
#define MSMeshletBuilder_hpp

#include <stdio.h>
#include "MeshClusterBuilder.hpp"

class MSMeshletBuilder : public MeshClusterBuilder
{
public:
    MSMeshletBuilder();
    ~MSMeshletBuilder();
    template<class IndexType>
    void Build(const Vector3f* pVertexData, const UInt32 nVertexDataCount, const UInt32* pIndexData, const UInt32 nIndexDataCount, const AABB bounds, int& nClusterCount, MeshCluster** pMeshCluster);
};

#endif /* MSMeshletBuilder_hpp */
