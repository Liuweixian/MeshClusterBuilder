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
        int Offset;
        int Num;
        std::vector<idx_t> Adjacency;
        std::vector<idx_t> AdjacencyCost;
        std::vector<idx_t> AdjacencyOffset;
    };
    // Inclusive
    struct Range
    {
        UInt32 Begin;
        UInt32 End;
        bool operator<(const Range& Other) const { return Begin < Other.Begin; }
    };
    std::vector<Range> m_Ranges;
    std::vector<UInt32> m_Indexes;
    
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
    UInt32 m_NumElements;
    int m_MinPartitionSize = 0;
    int m_MaxPartitionSize = 0;
    UInt32 m_NumPartitions;
    std::vector<idx_t> m_PartitionIDs;
    std::vector<int> m_SwappedWith;
    std::vector<UInt32> m_SortedTo;
    std::multimap<UInt32, UInt32> m_LocalityLinks;
};

inline void GraphPartitioner::AddAdjacency(GraphData* graph, UInt32 adjIndex, UInt32 cost)
{
    graph->Adjacency.push_back(m_SortedTo[adjIndex]);
    graph->AdjacencyCost.push_back(cost);
}

inline void GraphPartitioner::AddLocalityLinks(GraphData* graph, UInt32 index, UInt32 cost)
{
    auto iter = m_LocalityLinks.equal_range(index);
    for (auto it = iter.first; it != iter.second; ++it)
    {
        UInt32 adjIndex = it->second;
        graph->Adjacency.push_back(m_SortedTo[adjIndex]);
        graph->AdjacencyCost.push_back(cost);
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
void GraphPartitioner::BuildLocalityLinks(DisjointSet& disjointSet, const MinMaxAABB& bounds, GetCenter& GetCenterFunc)
{
    std::vector<UInt32> sortKeys(m_NumElements);
    m_SortedTo.resize(m_NumElements);
    //const bool singleThreaded = m_NumElements < 5000;
    for (int index = 0; index != m_NumElements; ++index)
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
   
    std::swap(m_SortedTo, m_Indexes);
    std::sort(m_SortedTo.begin(), m_SortedTo.end(),
        [&](UInt32 Index, UInt32 index2)
    {
        return sortKeys[Index] < sortKeys[index2];
    });
    sortKeys.clear();
    
    std::swap(m_Indexes, m_SortedTo);
    for (UInt32 i = 0; i < m_NumElements; i++)
    {
        m_SortedTo[m_Indexes[i]] = i;
    }
    std::vector<Range> islandRuns;
    islandRuns.resize(m_NumElements);
    // Run length acceleration
    // Range of identical IslandID denoting that elements are connected.
    // Used for jumping past connected elements to the next nearby disjoint element.
    {
        UInt32 runIslandID = 0;
        UInt32 runFirstElement = 0;
        for (UInt32 i = 0; i < m_NumElements; i++)
        {
            UInt32 islandID = disjointSet.Find(m_Indexes[i]);
            if (runIslandID != islandID)
            {
                // We found the end so rewind to the beginning of the run and fill.
                for (UInt32 j = runFirstElement; j < i; j++)
                {
                    islandRuns[j].End = i - 1;
                }
                // Start the next run
                runIslandID = islandID;
                runFirstElement = i;
            }
            islandRuns[i].Begin = runFirstElement;
        }
        // Finish the last run
        for (UInt32 j = runFirstElement; j < m_NumElements; j++)
        {
            islandRuns[j].End = m_NumElements - 1;
        }
    }
    for (UInt32 i = 0; i < m_NumElements; i++)
    {
        UInt32 index = m_Indexes[i];
        UInt32 runLength = islandRuns[i].End - islandRuns[i].Begin + 1;
        if (runLength < 128)
        {
            UInt32 islandID = disjointSet[index];
            Vector3f center = GetCenterFunc(index);
            UInt32 closestIndex[3] = { ~0u, ~0u, ~0u };
            float  closestDist2[3] = { __FLT_MAX__, __FLT_MAX__, __FLT_MAX__ }; 
            for (int direction = 0; direction < 2; direction++)
            {
                UInt32 limit = direction ? m_NumElements - 1 : 0;
                UInt32 step = direction ? 1 : -1;
                UInt32 adj = i;
                for (int iterations = 0; iterations < 16; iterations++)
                {
                    if (adj == limit)
                        break;
                    adj += step;
                    
                    UInt32 adjIndex = m_Indexes[adj];
                    UInt32 adjIslandID = disjointSet[adjIndex];
                    if (islandID == adjIslandID)
                    {
                        // Skip past this run
                        if (direction)
                            adj = islandRuns[adj].End;
                        else
                            adj = islandRuns[adj].Begin;
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
                    m_LocalityLinks.insert(std::make_pair(index, closestIndex[k]));
                    m_LocalityLinks.insert(std::make_pair(closestIndex[k], index));
                }
            }
        }
    }
 
}


#endif /* GraphPartitioner_hpp */
