//
//  GraphPartitioner.cpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#include "GraphPartitioner.hpp"

GraphPartitioner::GraphPartitioner(UInt32 num) : m_NumElements(num)
{
    m_Indexes.reserve(m_NumElements);
    for (UInt32 i = 0; i < m_NumElements; i++)
    {
        m_Indexes.push_back(i);
    }
}

GraphPartitioner::GraphData* GraphPartitioner::NewGraph(UInt32 adjacencyCount) const
{
    adjacencyCount += m_LocalityLinks.size();
    GraphData *graph = new GraphPartitioner::GraphData;
    graph->Offset = 0;
    graph->Num = m_NumElements;
    graph->Adjacency.reserve(adjacencyCount);
    graph->AdjacencyCost.reserve(adjacencyCount);
    graph->AdjacencyOffset.resize(m_NumElements + 1);
    return graph;
}

void GraphPartitioner::Partition(GraphData* graph, int InMinPartitionSize, int InMaxPartitionSize)
{
    m_MinPartitionSize = InMinPartitionSize;
    m_MaxPartitionSize = InMaxPartitionSize;
    const int targetPartitionSize = (m_MinPartitionSize + m_MaxPartitionSize) / 2;
    const int targetNumPartitions = (graph->Num + targetPartitionSize - 1) / targetPartitionSize;//FMath::DivideAndRoundUp(Graph->Num, TargetPartitionSize);
    if (targetNumPartitions > 1)
    {
        m_PartitionIDs.insert(m_PartitionIDs.end(), m_NumElements, 0);
        
        idx_t numConstraints = 1;
        idx_t partsCount = targetNumPartitions;
        idx_t edgesCut = 0;
        
        idx_t options[METIS_NOPTIONS];
        METIS_SetDefaultOptions(options);
        options[METIS_OPTION_UFACTOR] = 200;//( 1000 * MaxPartitionSize * TargetNumPartitions ) / NumElements - 1000;
        //Options[ METIS_OPTION_NCUTS ] = 8;
        //Options[ METIS_OPTION_IPTYPE ] = METIS_IPTYPE_RANDOM;
        //Options[ METIS_OPTION_SEED ] = 17;
        
        int r = METIS_PartGraphKway(
            &graph->Num,
            &numConstraints,            // number of balancing constraints
            graph->AdjacencyOffset.data(),
            graph->Adjacency.data(),
            NULL,                        // Vert weights
            NULL,                        // Vert sizes for computing the total communication volume
            graph->AdjacencyCost.data(),    // Edge weights
            &partsCount,
            NULL,                        // Target partition weight
            NULL,                        // Allowed load imbalance tolerance
            options,
            &edgesCut,
            m_PartitionIDs.data()
        );
        
        if (r == METIS_OK)
        {
            std::vector<UInt32> elementCount;
            elementCount.insert(elementCount.end(), targetNumPartitions, 0);
            
            for (UInt32 i = 0; i < m_NumElements; i++)
            {
                elementCount[m_PartitionIDs[i]]++;
            }
            
            UInt32 begin = 0;
            m_Ranges.insert(m_Ranges.end(), targetNumPartitions, Range());
            for (int PartitionIndex = 0; PartitionIndex < targetNumPartitions; PartitionIndex++)
            {
                m_Ranges[PartitionIndex] = { begin, begin + elementCount[PartitionIndex] };
                begin += elementCount[PartitionIndex];
                elementCount[PartitionIndex] = 0;
            }
            
            std::vector<UInt32> oldIndexes;
            std::swap(m_Indexes, oldIndexes);
            m_Indexes.insert(m_Indexes.end(), m_NumElements, 0);
            for (UInt32 i = 0; i < m_NumElements; i++)
            {
                UInt32 partitionIndex = m_PartitionIDs[i];
                UInt32 Offset = m_Ranges[partitionIndex].Begin;
                UInt32 Num = elementCount[partitionIndex]++;
                m_Indexes[Offset + Num] = oldIndexes[i];
            }
            m_PartitionIDs.clear();
        }
    }
    else
    {
        // Single
        m_Ranges.push_back({ 0, m_NumElements });
    }
}

void GraphPartitioner::BisectGraph(GraphData* graph, GraphData* childGraphs[2])
{
    childGraphs[0] = nullptr;
    childGraphs[1] = nullptr;
    auto AddPartition =
        [this](int offset, int num)
    {
        Range& range = m_Ranges[m_NumPartitions++];
        range.Begin = offset;
        range.End = offset + num;
    };
    
    if (graph->Num <= m_MaxPartitionSize)
    {
        AddPartition(graph->Offset, graph->Num);
        return;
    }
    
    const int targetPartitionSize = (m_MinPartitionSize + m_MaxPartitionSize) / 2;
    const int targetNumPartitions = std::max(2, (graph->Num + targetPartitionSize - 1) / targetPartitionSize);
    assert(graph->AdjacencyOffset.size() == (graph->Num + 1));
    
    idx_t numConstraints = 1;
    idx_t numParts = 2;
    idx_t edgesCut = 0;
    
    real_t partitionWeights[] = {
        float(targetNumPartitions / 2) / targetNumPartitions,
        1.0f - float(targetNumPartitions / 2) / targetNumPartitions
    };
    
    idx_t options[METIS_NOPTIONS];
    METIS_SetDefaultOptions(options);
    
    // Allow looser tolerance when at the higher levels. Strict balance isn't that important until it gets closer to partition sized.
    bool bLoose = targetNumPartitions >= 128 || m_MaxPartitionSize / m_MinPartitionSize > 1;
    //bool bSlow = graph->Num < 4096;
    
    options[METIS_OPTION_UFACTOR] = bLoose ? 200 : 1;
    //Options[ METIS_OPTION_NCUTS ] = Graph->Num < 1024 ? 8 : ( Graph->Num < 4096 ? 4 : 1 );
    //Options[ METIS_OPTION_NCUTS ] = bSlow ? 4 : 1;
    //Options[ METIS_OPTION_NITER ] = bSlow ? 20 : 10;
    //Options[ METIS_OPTION_IPTYPE ] = METIS_IPTYPE_RANDOM;
    //Options[ METIS_OPTION_MINCONN ] = 1;
    
    int r = METIS_PartGraphRecursive(
        &graph->Num,
        &numConstraints,            // number of balancing constraints
        graph->AdjacencyOffset.data(),
        graph->Adjacency.data(),
        NULL,                        // Vert weights
        NULL,                        // Vert sizes for computing the total communication volume
        graph->AdjacencyCost.data(),    // Edge weights
        &numParts,
        partitionWeights,            // Target partition weight
        NULL,                        // Allowed load imbalance tolerance
        options,
        &edgesCut,
        m_PartitionIDs.data() + graph->Offset
    );
    
    if ((r == METIS_OK))
    {
        // In place divide the array
        // Both sides remain sorted but back is reversed.
        int front = graph->Offset;
        int back = graph->Offset + graph->Num - 1;
        while (front <= back)
        {
            while (front <= back && m_PartitionIDs[front] == 0)
            {
                m_SwappedWith[front] = front;
                front++;
            }
            while (front <= back && m_PartitionIDs[back] == 1)
            {
                m_SwappedWith[back] = back;
                back--;
            }
            if (front < back)
            {
                std::swap(m_Indexes[front], m_Indexes[back]);
                m_SwappedWith[front] = back;
                m_SwappedWith[back] = front;
                front++;
                back--;
            }
        }
        
        int split = front;
        
        int num[2];
        num[0] = split - graph->Offset;
        num[1] = graph->Offset + graph->Num - split;
        
        assert(num[0] > 1);
        assert(num[1] > 1);
        if (num[0] <= m_MaxPartitionSize && num[1] <= m_MaxPartitionSize)
        {
            AddPartition(graph->Offset, num[0]);
            AddPartition(split, num[1]);
        }
        else
        {
            for (int i = 0; i < 2; i++)
            {
                childGraphs[i] = new GraphData;
                childGraphs[i]->Adjacency.reserve(graph->Adjacency.size() >> 1);
                childGraphs[i]->AdjacencyCost.reserve(graph->Adjacency.size() >> 1);
                childGraphs[i]->AdjacencyOffset.reserve(num[i] + 1);
                childGraphs[i]->Num = num[i];
            }
            childGraphs[0]->Offset = graph->Offset;
            childGraphs[1]->Offset = split;
            
            for (int i = 0; i < graph->Num; i++)
            {
                GraphData* childGraph = childGraphs[i >= childGraphs[0]->Num];
                childGraph->AdjacencyOffset.push_back((idx_t)childGraph->Adjacency.size());
                int index = m_SwappedWith[graph->Offset + i] - graph->Offset;
                for (UInt32 AdjIndex = graph->AdjacencyOffset[index]; AdjIndex < graph->AdjacencyOffset[index + 1]; AdjIndex++)
                {
                    UInt32 adj = graph->Adjacency[AdjIndex];
                    UInt32 adjCost = graph->AdjacencyCost[AdjIndex];
                    
                    // Remap to child
                    adj = m_SwappedWith[graph->Offset + adj] - childGraph->Offset;
                    
                    // Edge connects to node in this graph
                    if (0 <= adj && adj < childGraph->Num)
                    {
                        childGraph->Adjacency.push_back(adj);
                        childGraph->AdjacencyCost.push_back(adjCost);
                    }
                }
            }
            childGraphs[0]->AdjacencyOffset.push_back((idx_t)childGraphs[0]->Adjacency.size());
            childGraphs[1]->AdjacencyOffset.push_back((idx_t)childGraphs[1]->Adjacency.size());
        }
    }
}
void GraphPartitioner::RecursiveBisectGraph(GraphData* graph)
{
    GraphData* childGraphs[2];
    BisectGraph(graph, childGraphs);
    delete graph;
    
    if (childGraphs[0] && childGraphs[1])
    {
        RecursiveBisectGraph(childGraphs[0]);
        RecursiveBisectGraph(childGraphs[1]);
    }
}

void GraphPartitioner::PartitionStrict(GraphData* graph, int inMinPartitionSize, int inMaxPartitionSize, bool bThreaded)
{
    m_MinPartitionSize = inMinPartitionSize;
    m_MaxPartitionSize = inMaxPartitionSize;
    
    m_PartitionIDs.insert(m_PartitionIDs.end(), m_NumElements, 0);
    m_SwappedWith.insert(m_SwappedWith.end(), m_NumElements, 0);
    
    // Adding to atomically so size big enough to not need to grow.
    int numPartitionsExpected = (graph->Num + m_MinPartitionSize - 1) / m_MinPartitionSize;//FMath::DivideAndRoundUp(Graph->Num, MinPartitionSize);
    m_Ranges.insert(m_Ranges.end(), numPartitionsExpected * 2, Range());
    m_NumPartitions = 0;
    
    {
        RecursiveBisectGraph(graph);
    }
    
    m_Ranges.resize(m_NumPartitions);
    
    m_PartitionIDs.clear();
    m_SwappedWith.clear();
}
