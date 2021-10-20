//
//  MSMeshletBuilder.cpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#include "MSMeshletBuilder.hpp"
#include <vector>
#include "Utilities.hpp"

MSMeshletBuilder::MSMeshletBuilder()
{
    m_nClusterSize = 64;
}

MSMeshletBuilder::~MSMeshletBuilder()
{
    
}

template<class IndexType>
void MSMeshletBuilder::Build(const Vector3f* pVertexData, const UInt32 nVertexDataCount, const IndexType* pIndexData, const UInt32 nIndexDataCount, const AABB bounds, int& nClusterCount, MeshCluster** pMeshCluster)
{
    const uint32_t triCount = nIndexDataCount / 3;
    
    // Build a primitive adjacency list
    std::vector<uint32_t> adjacency;
    adjacency.resize(nIndexDataCount);

    BuildAdjacencyList(pIndexData, nIndexDataCount, pVertexData, nVertexDataCount, adjacency.data());
}

template void MSMeshletBuilder::Build<UInt32>(const Vector3f *pVertexData, const UInt32 nVertexDataCount, const UInt32 *pIndexData, const UInt32 nIndexDataCount, const AABB bounds, int &nClusterCount, MeshCluster **pMeshCluster);
