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
#include "MeshClusterResult.h"


class UEMetisMeshClusterBuilder : public MeshClusterBuilder
{
public:
    UEMetisMeshClusterBuilder();
    ~UEMetisMeshClusterBuilder();
    template<class IndexType>
    void Build(const Vector3f* pVertexData, const UInt32 nVertexDataCount, const IndexType* pIndexData, const UInt32 nIndexDataCount, const MinMaxAABB bounds, MeshClusterResult* pMeshClusterResult);
};

#endif /* UEMetisMeshClusterBuilder_hpp */
