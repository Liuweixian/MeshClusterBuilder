//
//  GraphPartitioner.cpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#include "GraphPartitioner.hpp"

GraphPartitioner::GraphPartitioner(UInt32 num) : m_numElements(num)
{
    m_indexes.reserve(m_numElements);
    for (UInt32 i = 0; i < m_numElements; i++)
    {
        m_indexes.push_back(i);
    }
}

GraphPartitioner::GraphData* GraphPartitioner::NewGraph(UInt32 adjacencyCount) const
{
    adjacencyCount += m_localityLinks.size();
    GraphData *graph = new GraphPartitioner::GraphData;
    graph->offset = 0;
    graph->count = m_numElements;
    graph->adjncy.reserve(adjacencyCount);
    graph->adjncyCost.reserve(adjacencyCount);
    graph->xadj.reserve(m_numElements + 1);
    return graph;
}

void GraphPartitioner::Partition(GraphData* graph, int InMinPartitionSize, int InMaxPartitionSize)
{
    m_minPartitionSize = InMinPartitionSize;
    m_maxPartitionSize = InMaxPartitionSize;
    const int targetPartitionSize = (m_minPartitionSize + m_maxPartitionSize) / 2;
    const int targetNumPartitions = (graph->count + targetPartitionSize - 1) / targetPartitionSize;//FMath::DivideAndRoundUp(Graph->Num, TargetPartitionSize);
    if (targetNumPartitions > 1)
    {
        m_partitionID.insert(m_partitionID.end(), m_numElements, 0);
        idx_t numConstraints = 1;
        idx_t partsCount = targetNumPartitions;
        idx_t edgesCut = 0;
        idx_t options[METIS_NOPTIONS];
        METIS_SetDefaultOptions(options);
        options[METIS_OPTION_UFACTOR] = 200;//( 1000 * MaxPartitionSize * TargetNumPartitions ) / NumElements - 1000;
        //Options[ METIS_OPTION_NCUTS ] = 8;
        //Options[ METIS_OPTION_IPTYPE ] = METIS_IPTYPE_RANDOM;
        //Options[ METIS_OPTION_SEED ] = 17;
        //int r = METIS_PartGraphRecursive(
        int r = METIS_PartGraphKway(
            &graph->count,
            &numConstraints,            // number of balancing constraints
            graph->xadj.data(),
            graph->adjncy.data(),
            NULL,                        // Vert weights
            NULL,                        // Vert sizes for computing the total communication volume
            graph->adjncyCost.data(),    // Edge weights
            &partsCount,
            NULL,                        // Target partition weight
            NULL,                        // Allowed load imbalance tolerance
            options,
            &edgesCut,
            m_partitionID.data()
        );
        if ((r == METIS_OK))
        {
            std::vector<UInt32> elementCount;
            elementCount.insert(elementCount.end(), targetNumPartitions, 0);
            for (UInt32 i = 0; i < m_numElements; i++)
            {
                elementCount[m_partitionID[i]]++;
            }
            UInt32 begin = 0;
            m_ranges.insert(m_ranges.end(), targetNumPartitions, Range());
            for (int PartitionIndex = 0; PartitionIndex < targetNumPartitions; PartitionIndex++)
            {
                m_ranges[PartitionIndex] = { begin, begin + elementCount[PartitionIndex] };
                begin += elementCount[PartitionIndex];
                elementCount[PartitionIndex] = 0;
            }
            std::vector<UInt32> oldIndexes;
            std::swap(m_indexes, oldIndexes);
            m_indexes.insert(m_indexes.end(), m_numElements, 0);
            for (UInt32 i = 0; i < m_numElements; i++)
            {
                UInt32 partitionIndex = m_partitionID[i];
                UInt32 Offset = m_ranges[partitionIndex].begin;
                UInt32 Num = elementCount[partitionIndex]++;
                m_indexes[Offset + Num] = oldIndexes[i];
            }
            m_partitionID.clear();
        }
    }
    else
    {
        // Single
        m_ranges.push_back({ 0, m_numElements });
    }
}

void GraphPartitioner::BisectGraph(GraphData* graph, GraphData* childGraphs[2])
{
    childGraphs[0] = nullptr;
    childGraphs[1] = nullptr;
    auto AddPartition =
        [this](int offset, int num)
    {
        Range& range = m_ranges[m_numPartitions++];
        range.begin = offset;
        range.end = offset + num;
    };
    if (graph->count <= m_maxPartitionSize)
    {
        AddPartition(graph->offset, graph->count);
        return;
    }
    const int targetPartitionSize = (m_minPartitionSize + m_maxPartitionSize) / 2;
    const int targetNumPartitions = std::max(2,
        (graph->count + targetPartitionSize - 1) / targetPartitionSize);
    assert(graph->xadj.size() == graph->count + 1);
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
    bool bLoose = targetNumPartitions >= 128 || m_maxPartitionSize / m_minPartitionSize > 1;
    bool bSlow = graph->count < 4096;
    options[METIS_OPTION_UFACTOR] = bLoose ? 200 : 1;
    //Options[ METIS_OPTION_NCUTS ] = Graph->Num < 1024 ? 8 : ( Graph->Num < 4096 ? 4 : 1 );
    //Options[ METIS_OPTION_NCUTS ] = bSlow ? 4 : 1;
    //Options[ METIS_OPTION_NITER ] = bSlow ? 20 : 10;
    //Options[ METIS_OPTION_IPTYPE ] = METIS_IPTYPE_RANDOM;
    //Options[ METIS_OPTION_MINCONN ] = 1;
    int r = METIS_PartGraphRecursive(
        &graph->count,
        &numConstraints,            // number of balancing constraints
        graph->xadj.data(),
        graph->adjncy.data(),
        NULL,                        // Vert weights
        NULL,                        // Vert sizes for computing the total communication volume
        graph->adjncyCost.data(),    // Edge weights
        &numParts,
        partitionWeights,            // Target partition weight
        NULL,                        // Allowed load imbalance tolerance
        options,
        &edgesCut,
        m_partitionID.data() + graph->offset
    );
    if ((r == METIS_OK))
    {
        // In place divide the array
        // Both sides remain sorted but back is reversed.
        int front = graph->offset;
        int back = graph->offset + graph->count - 1;
        while (front <= back)
        {
            while (front <= back && m_partitionID[front] == 0)
            {
                m_swappedWith[front] = front;
                front++;
            }
            while (front <= back && m_partitionID[back] == 1)
            {
                m_swappedWith[back] = back;
                back--;
            }
            if (front < back)
            {
                std::swap(m_indexes[front], m_indexes[back]);
                m_swappedWith[front] = back;
                m_swappedWith[back] = front;
                front++;
                back--;
            }
        }
        int split = front;
        int num[2];
        num[0] = split - graph->offset;
        num[1] = graph->offset + graph->count - split;
        assert(num[0] > 1);
        assert(num[1] > 1);
        if (num[0] <= m_maxPartitionSize && num[1] <= m_maxPartitionSize)
        {
            AddPartition(graph->offset, num[0]);
            AddPartition(split, num[1]);
        }
        else
        {
            for (int i = 0; i < 2; i++)
            {
                childGraphs[i] = new GraphData;
                childGraphs[i]->adjncy.reserve(graph->adjncy.size() >> 1);
                childGraphs[i]->adjncyCost.reserve(graph->adjncy.size() >> 1);
                childGraphs[i]->xadj.reserve(num[i] + 1);
                childGraphs[i]->count = num[i];
            }
            childGraphs[0]->offset = graph->offset;
            childGraphs[1]->offset = split;
            for (int i = 0; i < graph->count; i++)
            {
                GraphData* childGraph = childGraphs[i >= childGraphs[0]->count];
                childGraph->xadj.push_back(childGraph->adjncy.size());
                int index = m_swappedWith[graph->offset + i] - graph->offset;
                for (UInt32 AdjIndex = graph->xadj[index]; AdjIndex < graph->xadj[index + 1]; AdjIndex++)
                {
                    UInt32 Adj = graph->adjncy[AdjIndex];
                    UInt32 adjCost = graph->adjncyCost[AdjIndex];
                    // Remap to child
                    Adj = m_swappedWith[graph->offset + Adj] - childGraph->offset;
                    // Edge connects to node in this graph
                    if (0 <= Adj && Adj < childGraph->count)
                    {
                        childGraph->adjncy.push_back(Adj);
                        childGraph->adjncyCost.push_back(adjCost);
                    }
                }
            }
            childGraphs[0]->xadj.push_back(childGraphs[0]->adjncy.size());
            childGraphs[1]->xadj.push_back(childGraphs[1]->adjncy.size());
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
    m_minPartitionSize = inMinPartitionSize;
    m_maxPartitionSize = inMaxPartitionSize;
    m_partitionID.insert(m_partitionID.end(), m_numElements, 0);
    m_swappedWith.insert(m_swappedWith.end(), m_numElements, 0);
    // Adding to atomically so size big enough to not need to grow.
    int numPartitionsExpected = (graph->count + m_minPartitionSize - 1) / m_minPartitionSize;//FMath::DivideAndRoundUp(Graph->Num, MinPartitionSize);
    m_ranges.insert(m_ranges.end(), numPartitionsExpected * 2, Range());
    m_numPartitions = 0;
    {
        RecursiveBisectGraph(graph);
    }
    m_ranges.reserve(m_numPartitions);
    m_partitionID.clear();
 
    m_swappedWith.clear();
}
