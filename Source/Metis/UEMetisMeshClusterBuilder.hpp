//
//  UEMetisMeshClusterBuilder.hpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#ifndef UEMetisMeshClusterBuilder_hpp
#define UEMetisMeshClusterBuilder_hpp

#include <stdio.h>
#include "MeshClusterBuilder.hpp"


class UEMetisMeshClusterBuilder : public MeshClusterBuilder
{
public:
    UEMetisMeshClusterBuilder();
    ~UEMetisMeshClusterBuilder();
    template<class IndexType>
    void Build(const Vector3f* pVertexData, const UInt32 nVertexDataCount, const IndexType* pIndexData, const UInt32 nIndexDataCount, const AABB bounds, int& nClusterCount, MeshCluster** pMeshCluster);
};

#endif /* UEMetisMeshClusterBuilder_hpp */
