//
//  MSMeshletBuilder.cpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#include "MSMeshletBuilder.hpp"
#include <unordered_set>
#include "Utilities.hpp"

MSMeshletBuilder::MSMeshletBuilder()
{
    m_nClusterSize = 64;
}

MSMeshletBuilder::~MSMeshletBuilder()
{
    
}

// Sort in reverse order to use vector as a queue with pop_back.
bool CompareScores(const std::pair<uint32_t, float>& a, const std::pair<uint32_t, float>& b)
{
    return a.second > b.second;
}

template<class IndexType>
void MSMeshletBuilder::Build(const Vector3f* pVertexData, const UInt32 nVertexDataCount, const IndexType* pIndexData, const UInt32 nIndexDataCount, const AABB bounds, int& nClusterCount, MeshCluster** pMeshCluster)
{
    const uint32_t triCount = nIndexDataCount / 3;
    
    // Build a primitive adjacency list
    std::vector<uint32_t> adjacency;
    adjacency.resize(nIndexDataCount);

    BuildAdjacencyList(pIndexData, nIndexDataCount, pVertexData, nVertexDataCount, adjacency.data());
    
    // Rest our outputs
    std::vector<InlineMeshlet<IndexType>> output;
    output.clear();
    output.emplace_back();
    auto* curr = &output.back();

    // Bitmask of all triangles in mesh to determine whether a specific one has been added.
    std::vector<bool> checklist;
    checklist.resize(triCount);

    std::vector<Vector3f> m_positions;
    std::vector<Vector3f> normals;
    std::vector<std::pair<uint32_t, float>> candidates;
    std::unordered_set<uint32_t> candidateCheck;

    Vector4f psphere, normal;

    // Arbitrarily start at triangle zero.
    uint32_t triIndex = 0;
    candidates.push_back(std::make_pair(triIndex, 0.0f));
    candidateCheck.insert(triIndex);
    
    // Continue adding triangles until
    while (!candidates.empty())
    {
        uint32_t index = candidates.back().first;
        candidates.pop_back();

        IndexType tri[3] =
        {
            pIndexData[index * 3],
            pIndexData[index * 3 + 1],
            pIndexData[index * 3 + 2],
        };

        assert(tri[0] < nVertexDataCount);
        assert(tri[1] < nVertexDataCount);
        assert(tri[2] < nVertexDataCount);

        // Try to add triangle to meshlet
        if (AddToMeshlet(nVertexDataCount, m_nClusterSize, *curr, tri))
        {
            // Success! Mark as added.
            checklist[index] = true;

            // Add m_positions & normal to list
            Vector3f points[3] =
            {
                pVertexData[tri[0]],
                pVertexData[tri[1]],
                pVertexData[tri[2]],
            };

            m_positions.push_back(points[0]);
            m_positions.push_back(points[1]);
            m_positions.push_back(points[2]);

            Vector3f Normal(ComputeNormal(points));
            normals.push_back(Normal);

            // Compute new bounding sphere & normal axis
            psphere = MinimumBoundingSphere(m_positions.data(), static_cast<uint32_t>(m_positions.size()));
            
            Vector4f nsphere = MinimumBoundingSphere(normals.data(), static_cast<uint32_t>(normals.size()));
            normal = Normalize(nsphere);

            // Find and add all applicable adjacent triangles to candidate list
            const uint32_t adjIndex = index * 3;

            uint32_t adj[3] =
            {
                adjacency[adjIndex],
                adjacency[adjIndex + 1],
                adjacency[adjIndex + 2],
            };

            for (uint32_t i = 0; i < 3u; ++i)
            {
                // Invalid triangle in adjacency slot
                if (adj[i] == -1)
                    continue;
                
                // Already processed triangle
                if (checklist[adj[i]])
                    continue;

                // Triangle already in the candidate list
                if (candidateCheck.count(adj[i]))
                    continue;

                candidates.push_back(std::make_pair(adj[i], __FLT_MAX__));
                candidateCheck.insert(adj[i]);
            }

            // Re-score remaining candidate triangles
            for (uint32_t i = 0; i < static_cast<uint32_t>(candidates.size()); ++i)
            {
                uint32_t candidate = candidates[i].first;

                IndexType triIndices[3] =
                {
                    pIndexData[candidate * 3],
                    pIndexData[candidate * 3 + 1],
                    pIndexData[candidate * 3 + 2],
                };

                assert(triIndices[0] < nVertexDataCount);
                assert(triIndices[1] < nVertexDataCount);
                assert(triIndices[2] < nVertexDataCount);

                Vector3f triVerts[3] =
                {
                    pVertexData[triIndices[0]],
                    pVertexData[triIndices[1]],
                    pVertexData[triIndices[2]],
                };

                candidates[i].second = ComputeScore(*curr, psphere, normal, triIndices, triVerts);
            }

            // Determine whether we need to move to the next meshlet.
            if (IsMeshletFull(nVertexDataCount, m_nClusterSize, *curr))
            {
                m_positions.clear();
                normals.clear();
                candidateCheck.clear();

                // Use one of our existing candidates as the next meshlet seed.
                if (!candidates.empty())
                {
                    candidates[0] = candidates.back();
                    candidates.resize(1);
                    candidateCheck.insert(candidates[0].first);
                }

                output.emplace_back();
                curr = &output.back();
            }
            else
            {
                std::sort(candidates.begin(), candidates.end(), &CompareScores);
            }
        }
        else
        {
            if (candidates.empty())
            {
                m_positions.clear();
                normals.clear();
                candidateCheck.clear();

                output.emplace_back();
                curr = &output.back();
            }
        }

        // Ran out of candidates; add a new seed candidate to start the next meshlet.
        if (candidates.empty())
        {
            while (triIndex < triCount && checklist[triIndex])
                ++triIndex;

            if (triIndex == triCount)
                break;

            candidates.push_back(std::make_pair(triIndex, 0.0f));
            candidateCheck.insert(triIndex);
        }
    }

    // The last meshlet may have never had any primitives added to it - in which case we want to remove it.
    if (output.back().PrimitiveIndices.empty())
    {
        output.pop_back();
    }
}

template void MSMeshletBuilder::Build<UInt32>(const Vector3f *pVertexData, const UInt32 nVertexDataCount, const UInt32 *pIndexData, const UInt32 nIndexDataCount, const AABB bounds, int &nClusterCount, MeshCluster **pMeshCluster);

// Compute number of triangle vertices already exist in the meshlet
template <typename T>
uint32_t ComputeReuse(const InlineMeshlet<T>& meshlet, T (&triIndices)[3])
{
    uint32_t count = 0;

    for (uint32_t i = 0; i < static_cast<uint32_t>(meshlet.UniqueVertexIndices.size()); ++i)
    {
        for (uint32_t j = 0; j < 3u; ++j)
        {
            if (meshlet.UniqueVertexIndices[i] == triIndices[j])
            {
                ++count;
            }
        }
    }

    return count;
}

// Computes a candidacy score based on spatial locality, orientational coherence, and vertex re-use within a meshlet.
template <typename T>
float MSMeshletBuilder::ComputeScore(const InlineMeshlet<T>& meshlet, Vector4f sphere, Vector4f normal, T (&triIndices)[3], Vector3f* triVerts)
{
    const float reuseWeight = 0.334f;
    const float locWeight = 0.333f;
    const float oriWeight = 0.333f;
    
    Vector3f sphereCenter(sphere.x, sphere.y, sphere.z);
    float sphereRadius = sphere.w;
    
    // Vertex reuse
    uint32_t reuse = ComputeReuse(meshlet, triIndices);
    float reuseScore = 1 - float(reuse) / 3.0f;

    // Distance from center point
    float maxSq = 0.0f;
    for (uint32_t i = 0; i < 3u; ++i)
    {
        Vector3f v = sphereCenter - triVerts[i];
        float tmpMaxSq = Dot(v, v);
        if (tmpMaxSq > maxSq)
            maxSq = tmpMaxSq;
    }
    float r2 = sphereRadius * sphereRadius;
    float locScore = log(maxSq / r2 + 1);

    // Angle between normal and meshlet cone axis
    Vector3f n = ComputeNormal(triVerts);
    Vector3f nor(normal.x, normal.y, normal.z);
    float d = Dot(n, nor);
    float oriScore = (-d + 1) / 2.0f;

    float b = reuseWeight * reuseScore + locWeight * locScore + oriWeight * oriScore;

    return b;
}

// Determines whether a candidate triangle can be added to a specific meshlet; if it can, does so.
template <typename T>
bool MSMeshletBuilder::AddToMeshlet(uint32_t maxVerts, uint32_t maxPrims, InlineMeshlet<T>& meshlet, T (&tri)[3])
{
    // Are we already full of vertices?
    if (meshlet.UniqueVertexIndices.size() == maxVerts)
        return false;

    // Are we full, or can we store an additional primitive?
    if (meshlet.PrimitiveIndices.size() == maxPrims)
        return false;

    static const uint32_t Undef = uint32_t(-1);
    uint32_t indices[3] = { Undef, Undef, Undef };
    uint32_t newCount = 3;

    for (uint32_t i = 0; i < meshlet.UniqueVertexIndices.size(); ++i)
    {
        for (uint32_t j = 0; j < 3; ++j)
        {
            if (meshlet.UniqueVertexIndices[i] == tri[j])
            {
                indices[j] = i;
                --newCount;
            }
        }
    }

    // Will this triangle fit?
    if (meshlet.UniqueVertexIndices.size() + newCount > maxVerts)
        return false;

    // Add unique vertex indices to unique vertex index list
    for (uint32_t j = 0; j < 3; ++j)
    {
        if (indices[j] == Undef)
        {
            indices[j] = static_cast<uint32_t>(meshlet.UniqueVertexIndices.size());
            meshlet.UniqueVertexIndices.push_back(tri[j]);
        }
    }

    // Add the new primitive
    typename InlineMeshlet<T>::PackedTriangle prim = {};
    prim.i0 = indices[0];
    prim.i1 = indices[1];
    prim.i2 = indices[2];

    meshlet.PrimitiveIndices.push_back(prim);

    return true;
}

template <typename T>
bool MSMeshletBuilder::IsMeshletFull(uint32_t maxVerts, uint32_t maxPrims, const InlineMeshlet<T>& meshlet)
{
    assert(meshlet.UniqueVertexIndices.size() <= maxVerts);
    assert(meshlet.PrimitiveIndices.size() <= maxPrims);

    return meshlet.UniqueVertexIndices.size() == maxVerts
        || meshlet.PrimitiveIndices.size() == maxPrims;
}

Vector3f MSMeshletBuilder::ComputeNormal(Vector3f* tri)
{
    Vector3f p0 = tri[0];
    Vector3f p1 = tri[1];
    Vector3f p2 = tri[2];

    Vector3f v01 = p0 - p1;
    Vector3f v02 = p0 - p2;

    return Normalize(Cross(v01, v02));
}
