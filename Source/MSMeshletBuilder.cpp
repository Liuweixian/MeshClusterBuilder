//
//  MSMeshletBuilder.cpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#include "MSMeshletBuilder.hpp"
MSMeshletBuilder::MSMeshletBuilder()
{
    m_nClusterSize = 64;
}

MSMeshletBuilder::~MSMeshletBuilder()
{
    
}

template<class IndexType>
void MSMeshletBuilder::Build(const Vector3f* pVertexData, const UInt32 nVertexDataCount, const UInt32* pIndexData, const UInt32 nIndexDataCount, const AABB bounds, int& nClusterCount, MeshCluster** pMeshCluster)
{
    nClusterCount = 5;
}

template void MSMeshletBuilder::Build<UInt32>(const Vector3f *pVertexData, const UInt32 nVertexDataCount, const UInt32 *pIndexData, const UInt32 nIndexDataCount, const AABB bounds, int &nClusterCount, MeshCluster **pMeshCluster);
