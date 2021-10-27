//
//  UEMetisMeshClusterBuilder.cpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#include "UEMetisMeshClusterBuilder.hpp"
#include "DisjointSet.h"
#include <map>
#include <vector>
#include <algorithm>
#include "GraphPartitioner.hpp"

using namespace std;

UEMetisMeshClusterBuilder::UEMetisMeshClusterBuilder()
{
    m_nClusterSize = 128;
}

UEMetisMeshClusterBuilder::~UEMetisMeshClusterBuilder()
{
    
}


inline UInt32 MurmurFinalize32(UInt32 hash)
{
   hash ^= hash >> 16;
   hash *= 0x85ebca6b;
   hash ^= hash >> 13;
   hash *= 0xc2b2ae35;
   hash ^= hash >> 16;
   return hash;
}


inline UInt32 Murmur32(std::initializer_list<UInt32> initList)
{
   UInt32 hash = 0;
   for (auto element : initList)
   {
       element *= 0xcc9e2d51;
       element = (element << 15) | (element >> (32 - 15));
       element *= 0x1b873593;
       hash ^= element;
       hash = (hash << 13) | (hash >> (32 - 13));
       hash = hash * 5 + 0xe6546b64;
   }
   return MurmurFinalize32(hash);
}

inline UInt32 Cycle3(UInt32 value)
{
 
    UInt32 valueMod3 = value % 3;
    UInt32 value1Mod3 = (1 << valueMod3) & 3;
    return value - valueMod3 + value1Mod3;
}

inline UInt32 HashPosition(const Vector3f& position)
{
    union { float f; UInt32 i; } x;
    union { float f; UInt32 i; } y;
    union { float f; UInt32 i; } z;
 
    x.f = position.x;
    y.f = position.y;
    z.f = position.z;
    return Murmur32({
        position.x == 0.0f ? 0u : x.i,
        position.y == 0.0f ? 0u : y.i,
        position.z == 0.0f ? 0u : z.i
        });
}

template<class IndexType>
void UEMetisMeshClusterBuilder::Build(const Vector3f* pVertexData, const UInt32 nVertexDataCount, const IndexType* pIndexData, const UInt32 nIndexDataCount, const MinMaxAABB bounds, MeshClusterResult* pMeshClusterResult)
{
    typedef std::multimap<UInt32, UInt32> EdgeHashTable;
    EdgeHashTable edgeHash;
    for (UInt32 i = 0; i != nIndexDataCount; ++i)
    {
 
        UInt32 vertIndex0 = pIndexData[i];
        UInt32 vertIndex1 = pIndexData[Cycle3(i)];
        const Vector3f& position0 = pVertexData[vertIndex0];
        const Vector3f& position1 = pVertexData[vertIndex1];
        UInt32 hash0 = HashPosition(position0);
        UInt32 hash1 = HashPosition(position1);
        UInt32 hash = Murmur32({ hash0, hash1 });
        edgeHash.insert(std::make_pair(hash, i));
    }
    
    std::vector<UInt32> sharedEdges(nIndexDataCount);
    std::vector<UInt32> boundaryEdges(nIndexDataCount);
    
    const int numBitsPerDWORD = 32;
    const int numDwords = (nIndexDataCount + numBitsPerDWORD - 1) / numBitsPerDWORD;
    for (int i = 0; i != numDwords; ++i)
    {
        const int numBits = std::min<int>(numBitsPerDWORD, nIndexDataCount - i * numBitsPerDWORD);
       
        UInt32 Mask = 1;
        UInt32 dword = 0;
        for (int bitIndex = 0; bitIndex < numBits; bitIndex++, Mask <<= 1)
        {
            int edgeIndex = i * numBitsPerDWORD + bitIndex;
            UInt32 vertIndex0 = pIndexData[edgeIndex];
            UInt32 vertIndex1 = pIndexData[Cycle3(edgeIndex)];
            const Vector3f& position0 = pVertexData[vertIndex0];
            const Vector3f& position1 = pVertexData[vertIndex1];
            UInt32 Hash0 = HashPosition(position0);
            UInt32 Hash1 = HashPosition(position1);
            UInt32 positionHash = Murmur32({ Hash1, Hash0 });
            // Find edge with opposite direction that shares these 2 verts.
            /*
                  /\
                 /  \
                o-<<-o
                o->>-o
                 \  /
                  \/
            */
            UInt32 foundEdge = ~0u;
            auto iter = edgeHash.equal_range(positionHash);
            for (auto it = iter.first; it != iter.second; ++it)
            {
                UInt32 otherEdgeIndex = it->second;
                UInt32 otherVertIndex0 = pIndexData[otherEdgeIndex];
                UInt32 otherVertIndex1 = pIndexData[Cycle3(otherEdgeIndex)];
                if (position0 == pVertexData[otherVertIndex1] &&
                    position1 == pVertexData[otherVertIndex0])
                {
                    // Found matching edge.
                    // Hash table is not in deterministic order. Find stable match not just first.
                    foundEdge = std::min(foundEdge, otherEdgeIndex);
                }
            }
            sharedEdges[edgeIndex] = foundEdge;
            if (foundEdge == ~0u)
            {
                dword |= Mask;
            }
        }
 
        if (dword)
        {
            boundaryEdges[i] = dword;
        }
    }
    
    DisjointSet set(nIndexDataCount / 3);
    for (UInt32 edgeIndex = 0, num = sharedEdges.size(); edgeIndex < num; edgeIndex++)
    {
        UInt32 otherEdgeIndex = sharedEdges[edgeIndex];
        if (otherEdgeIndex != ~0u)
        {
            // OtherEdgeIndex is smallest that matches EdgeIndex
            // ThisEdgeIndex is smallest that matches OtherEdgeIndex
            UInt32 thisEdgeIndex = sharedEdges[otherEdgeIndex];
            assert(thisEdgeIndex != ~0u);
            assert(thisEdgeIndex <= edgeIndex);
            if (edgeIndex > thisEdgeIndex)
            {
                // Previous element points to OtherEdgeIndex
                sharedEdges[edgeIndex] = ~0u;
            }
            else if (edgeIndex > otherEdgeIndex)
            {
                // Second time seeing this
                set.UnionSequential(edgeIndex / 3, otherEdgeIndex / 3);
            }
        }
    }
    
    UInt32 triangleCount = nIndexDataCount / 3;
    GraphPartitioner graphPartitioner(triangleCount);
    {
        auto GetCenter = [&pVertexData, &pIndexData](UInt32 TriIndex)
        {
            Vector3f center;
            center = pVertexData[pIndexData[TriIndex * 3 + 0]];
            center += pVertexData[pIndexData[TriIndex * 3 + 1]];
            center += pVertexData[pIndexData[TriIndex * 3 + 2]];
            return center * (1.0f / 3.0f);
        };
        graphPartitioner.BuildLocalityLinks(set, bounds, GetCenter);
        auto *graph = graphPartitioner.NewGraph(nIndexDataCount);
        for (UInt32 i = 0; i < triangleCount; i++)
        {
            graph->AdjacencyOffset[i] = (idx_t)graph->Adjacency.size();
            UInt32 triIndex = graphPartitioner.m_Indexes[i];
            for (int k = 0; k < 3; k++)
            {
                UInt32 edgeIndex = sharedEdges[3 * triIndex + k];
                if (edgeIndex != ~0u)
                {
                    graphPartitioner.AddAdjacency(graph, edgeIndex / 3, 4 * 65);
                }
            }
            graphPartitioner.AddLocalityLinks(graph, triIndex, 1);
        }
        graph->AdjacencyOffset[triangleCount] = (UInt32)graph->Adjacency.size();
        graphPartitioner.PartitionStrict(graph, m_nClusterSize - 4, m_nClusterSize, true);
        assert(graphPartitioner.m_Ranges.size());
    }
    pMeshClusterResult->m_nCount = (int)graphPartitioner.m_Ranges.size();
    pMeshClusterResult->m_pMeshClusterList = new MeshCluster[pMeshClusterResult->m_nCount];
    for (int i = 0; i < pMeshClusterResult->m_nCount; i++)
    {
        const GraphPartitioner::Range& range = graphPartitioner.m_Ranges[i];
        int triangleCount = range.End - range.Begin;
        MeshCluster* meshCluster = new MeshCluster();
        meshCluster->m_nIndexCount = triangleCount * 3;
        meshCluster->m_pIndexBuffer = new UInt32[meshCluster->m_nIndexCount];
        int index = 0;
        for (int j = range.Begin; j < range.End; j++)
        {
            UInt32 triIndex = graphPartitioner.m_Indexes[j];
            for (int k = 0; k < 3; k++)
            {
                UInt32 vertIndex = pIndexData[triIndex * 3 + k];
                meshCluster->m_pIndexBuffer[index++] = vertIndex;
            }
        }
        pMeshClusterResult->m_pMeshClusterList[i] = meshCluster;
    }
}

template void UEMetisMeshClusterBuilder::Build<UInt32>(const Vector3f *pVertexData, const UInt32 nVertexDataCount, const UInt32 *pIndexData, const UInt32 nIndexDataCount, const MinMaxAABB bounds, MeshClusterResult* pMeshClusterResult);
