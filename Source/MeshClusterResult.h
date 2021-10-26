//
//  MeshClusterResult.h
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/26.
//

#ifndef MeshClusterResult_h
#define MeshClusterResult_h

struct MeshCluster
{
    MeshCluster() {}
    MeshCluster(const MeshCluster* v)
    {
        m_nIndexCount = v->m_nIndexCount;
        m_pIndexBuffer = v->m_pIndexBuffer;
    }
    int m_nIndexCount;
    UInt32* m_pIndexBuffer;
};

struct MeshClusterResult
{
    int m_nCount;
    MeshCluster* m_pMeshClusterList;
};
#endif /* MeshClusterResult_h */
