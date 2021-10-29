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
    MeshCluster()
    {
        m_nIndexCount = 0;
        m_pIndexBuffer = NULL;
    }
    
    MeshCluster(int indexCount)
    {
        m_nIndexCount = indexCount;
        m_pIndexBuffer = new UInt32[m_nIndexCount];
    }
    
    MeshCluster(const MeshCluster*& v)
    {
        m_nIndexCount = v->m_nIndexCount;
        m_pIndexBuffer = v->m_pIndexBuffer;
    }
    
    ~MeshCluster()
    {
        m_nIndexCount = 0;
        if (m_pIndexBuffer != NULL)
        {
            delete[] m_pIndexBuffer;
            m_pIndexBuffer = NULL;
        }
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
