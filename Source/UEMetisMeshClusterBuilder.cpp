//
//  UEMetisMeshClusterBuilder.cpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#include "UEMetisMeshClusterBuilder.hpp"

UEMetisMeshClusterBuilder::UEMetisMeshClusterBuilder()
{
    m_nClusterSize = 128;
}

UEMetisMeshClusterBuilder::~UEMetisMeshClusterBuilder()
{
    
}

template<class IndexType>
void UEMetisMeshClusterBuilder::Build(const Vector3f* pVertexData, const UInt32 nVertexDataCount, const UInt32* pIndexData, const UInt32 nIndexDataCount, const AABB bounds, int& nClusterCount, MeshCluster** pMeshCluster)
{
    nClusterCount = 5;
}

template void UEMetisMeshClusterBuilder::Build<UInt32>(const Vector3f *pVertexData, const UInt32 nVertexDataCount, const UInt32 *pIndexData, const UInt32 nIndexDataCount, const AABB bounds, int &nClusterCount, MeshCluster **pMeshCluster);
