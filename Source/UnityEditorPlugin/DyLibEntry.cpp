//
//  DyLibEntry.cpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#include <stdio.h>
#include "IUnityInterface.h"
#include "UEMetisMeshClusterBuilder.hpp"
#include "MSMeshletBuilder.hpp"
#include "MeshClusterResult.h"

static IUnityInterfaces* s_UnityInterfaces = NULL;
IUnityInterfaces& GetUnityInterfaces() { return *s_UnityInterfaces; }

typedef void(*UnityLogCallback)(const char* message);
static UnityLogCallback s_UnityLogCallback = nullptr;

enum BuilderType
{
    eUE_Metis,
    eMS_Meshlet
};


extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    s_UnityInterfaces = unityInterfaces;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RegisterUnityLogCallback(UnityLogCallback cb)
{
    s_UnityLogCallback = cb;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API BuildCluster(BuilderType eBuildType, const UInt32 nClusterSize, const Vector3f* pVertexData, const UInt32 nVertexDataCount, const UInt32* pIndexData, const UInt32 nIndexDataCount, const AABB bounds, MeshClusterResult* pMeshClusterResult)
{
    s_UnityLogCallback("Start Building Cluster...");
    if (eBuildType == eUE_Metis)
    {
        UEMetisMeshClusterBuilder metisBuilder;
        metisBuilder.SetClusterSize(nClusterSize);
        MinMaxAABB minMaxAABB(bounds);
        metisBuilder.Build<UInt32>(pVertexData, nVertexDataCount, pIndexData, nIndexDataCount, minMaxAABB, pMeshClusterResult);
    }
    else if (eBuildType == eMS_Meshlet)
    {
        MSMeshletBuilder meshletBuilder;
        meshletBuilder.SetClusterSize(nClusterSize);
        meshletBuilder.Build<UInt32>(pVertexData, nVertexDataCount, pIndexData, nIndexDataCount, bounds, pMeshClusterResult);
    }
    else
    {
        s_UnityLogCallback("Invalid Builder Type...");
    }
    s_UnityLogCallback("Finish Building Cluster...");
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ReleaseCulsterResult(MeshClusterResult* pMeshClusterResult)
{
    pMeshClusterResult->m_nCount = 0;
    if (pMeshClusterResult->m_pMeshClusterList != NULL)
    {
        delete[] pMeshClusterResult->m_pMeshClusterList;
        pMeshClusterResult->m_pMeshClusterList = NULL;
    }
}
