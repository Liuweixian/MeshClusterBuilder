//
//  GraphPartitioner.hpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#ifndef GraphPartitioner_hpp
#define GraphPartitioner_hpp

#include <stdio.h>
#include "TypeDef.h"
#include "DisjointSet.h"
#include "AABB.h"
#include "metis.h"
#include <map>

class GraphPartitioner
{
public:
    struct GraphData
    {
        int    offset;
        int    count;
        std::vector<idx_t>    adjncy;
        std::vector<idx_t>    adjncyCost;
        std::vector<idx_t>    xadj;
    };
    // Inclusive
    struct Range
    {
        UInt32    begin;
        UInt32    end;
        bool operator<(const Range& Other) const { return begin < Other.begin; }
    };
    std::vector<Range> m_ranges;
    std::vector<UInt32> m_indexes;
    
public:
    GraphPartitioner(UInt32 num);
    GraphData* NewGraph(UInt32 adjacencyCount) const;
    void        AddAdjacency(GraphData* Graph, UInt32 AdjIndex, UInt32 Cost);
    void        AddLocalityLinks(GraphData* Graph, UInt32 Index, UInt32 Cost);
    template< typename FGetCenter >
    void        BuildLocalityLinks(DisjointSet& DisjointSet, const MinMaxAABB& Bounds, FGetCenter& GetCenter);
    void        Partition(GraphData* Graph, int InMinPartitionSize, int InMaxPartitionSize);
    void        PartitionStrict(GraphData* Graph, int InMinPartitionSize, int InMaxPartitionSize, bool bThreaded);
    
private:
    void        BisectGraph(GraphData* Graph, GraphData* ChildGraphs[2]);
    void        RecursiveBisectGraph(GraphData* Graph);
    UInt32 m_numElements;
    int m_minPartitionSize = 0;
    int m_maxPartitionSize = 0;
    UInt32 m_numPartitions;
    std::vector<idx_t> m_partitionID;
    std::vector<int> m_swappedWith;
    std::vector<UInt32> m_sortedTo;
    std::multimap<UInt32, UInt32> m_localityLinks;
};

inline void GraphPartitioner::AddAdjacency(GraphData* graph, UInt32 adjIndex, UInt32 cost)
{
    graph->adjncy.push_back(m_sortedTo[adjIndex]);
    graph->adjncyCost.push_back(cost);
}

inline void GraphPartitioner::AddLocalityLinks(GraphData* graph, UInt32 index, UInt32 cost)
{
    auto iter = m_localityLinks.equal_range(index);
    for (auto it = iter.first; it != iter.second; ++it)
    {
        UInt32 adjIndex = it->second;
        graph->adjncy.push_back(m_sortedTo[adjIndex]);
        graph->adjncyCost.push_back(cost);
    }
}

inline UInt32 MortonCode3(UInt32 x)
{
    x &= 0x000003ff;
    x = (x ^ (x << 16)) & 0xff0000ff;
    x = (x ^ (x << 8)) & 0x0300f00f;
    x = (x ^ (x << 4)) & 0x030c30c3;
    x = (x ^ (x << 2)) & 0x09249249;
    return x;
}

template< typename GetCenter >
void GraphPartitioner::BuildLocalityLinks(DisjointSet& set, const MinMaxAABB& bounds, GetCenter& GetCenterFunc)
{
    std::vector<UInt32> sortKeys;
    sortKeys.reserve(m_numElements);
    m_sortedTo.reserve(m_numElements);
    const bool singleThreaded = m_numElements < 5000;
    for (int index = 0; index != m_numElements; ++index)
    {
        Vector3f center = GetCenterFunc(index);
        Vector3f centerLocal = (center - bounds.GetMin());
        Vector3f devide = (bounds.GetMax() - bounds.GetMin());
        centerLocal.x = centerLocal.x / devide.x;
        centerLocal.y = centerLocal.y / devide.y;
        centerLocal.z = centerLocal.z / devide.z;
        UInt32 morton;
        morton = MortonCode3(centerLocal.x * 1023);
        morton |= MortonCode3(centerLocal.y * 1023) << 1;
        morton |= MortonCode3(centerLocal.z * 1023) << 2;
        sortKeys[index] = morton;
    }
   
    m_sortedTo = m_indexes;
    std::sort(m_sortedTo.begin(), m_sortedTo.end(),
        [&](UInt32 Index, UInt32 index2)
    {
        return sortKeys[Index] < sortKeys[index2];
    });
    sortKeys.clear();
    std::swap(m_indexes, m_sortedTo);
    for (UInt32 i = 0; i < m_numElements; i++)
    {
        m_sortedTo[m_indexes[i]] = i;
    }
    std::vector<Range> islandRuns;
    islandRuns.reserve(m_numElements);
    // Run length acceleration
    // Range of identical IslandID denoting that elements are connected.
    // Used for jumping past connected elements to the next nearby disjoint element.
    {
        UInt32 runIslandID = 0;
        UInt32 runFirstElement = 0;
        for (UInt32 i = 0; i < m_numElements; i++)
        {
            UInt32 islandID = set.Find(m_indexes[i]);
            if (runIslandID != islandID)
            {
                // We found the end so rewind to the beginning of the run and fill.
                for (UInt32 j = runFirstElement; j < i; j++)
                {
                    islandRuns[j].end = i - 1;
                }
                // Start the next run
                runIslandID = islandID;
                runFirstElement = i;
            }
            islandRuns[i].begin = runFirstElement;
        }
        // Finish the last run
        for (UInt32 j = runFirstElement; j < m_numElements; j++)
        {
            islandRuns[j].end = m_numElements - 1;
        }
    }
    for (UInt32 i = 0; i < m_numElements; i++)
    {
        UInt32 index = m_indexes[i];
        UInt32 runLength = islandRuns[i].end - islandRuns[i].begin + 1;
        if (runLength < 128)
        {
            UInt32 islandID = set[index];
            Vector3f center = GetCenterFunc(index);
            UInt32 closestIndex[3] = { ~0u, ~0u, ~0u };
            float  closestDist2[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
            for (int direction = 0; direction < 2; direction++)
            {
                UInt32 limit = direction ? m_numElements - 1 : 0;
                UInt32 step = direction ? 1 : -1;
                UInt32 adj = i;
                for (int iterations = 0; iterations < 16; iterations++)
                {
                    if (adj == limit)
                        break;
                    adj += step;
                    UInt32 adjIndex = m_indexes[adj];
                    UInt32 adjIslandID = set[adjIndex];
                    if (islandID == adjIslandID)
                    {
                        // Skip past this run
                        if (direction)
                            adj = islandRuns[adj].end;
                        else
                            adj = islandRuns[adj].begin;
                    }
                    else
                    {
                        // Add to sorted list
                        Vector3f center2 = GetCenterFunc(adjIndex);
                        Vector3f dir = center - center2;
                        float adjDist2 = Dot(dir, dir);
                        for (int k = 0; k < 3; k++)
                        {
                            if (adjDist2 < closestDist2[k])
                            {
                                std::swap(adjIndex, closestIndex[k]);
                                std::swap(adjDist2, closestDist2[k]);
                            }
                        }
                    }
                }
            }
            for (int k = 0; k < 3; k++)
            {
                if (closestIndex[k] != ~0u)
                {
                    // Add both directions
                    m_localityLinks.insert(std::make_pair(index, closestIndex[k]));
                    m_localityLinks.insert(std::make_pair(closestIndex[k], index));
                }
            }
        }
    }
 
}


#endif /* GraphPartitioner_hpp */
