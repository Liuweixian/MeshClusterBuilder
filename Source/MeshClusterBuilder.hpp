//
//  MeshClusterBuilder.hpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/9/30.
//

#ifndef MeshClusterBuilder_hpp
#define MeshClusterBuilder_hpp

#include <stdio.h>
#include "Vector3.h"
#include "AABB.h"
#include "TypeDef.h"
#include "MeshCluster.hpp"

enum BuilderType { eUE_Metis, eMS_Meshlet };

class MeshClusterBuilder
{
public:
    MeshClusterBuilder();
    ~MeshClusterBuilder();
    
    template<class IndexType>
    void Build(BuilderType eBuildType, const Vector3f* pVertexData, const UInt32 nVertexDataCount, const UInt32* pIndexData, const UInt32 nIndexDataCount, const MinMaxAABB bounds, int& nClusterCount, MeshCluster** pMeshCluster);
private:
    int m_nClusterSize;
};

#endif /* MeshClusterBuilder_hpp */
