using System;
using System.Runtime.InteropServices;
using UnityEditor;
using UnityEngine;

public class MeshClusterPlugins
{
    enum BuilderType
    {
        eUE_Metis,
        eMS_Meshlet
    };

    struct MeshCluster
    {
        private int indexCount;
        private byte[] indexBuffer;
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] 
    public delegate void DebugLogCallback(string str);
    
    [DllImport("MeshClusterBuilder", CallingConvention = CallingConvention.Cdecl)]
    static extern void RegisterUnityLogCallback(DebugLogCallback cb);
    public static void UnityLogCallback(string log)
    {
        Debug.Log(log);
    }
    
    
    [DllImport("MeshClusterBuilder", CallingConvention = CallingConvention.Cdecl)]
    static extern void BuildCluster(BuilderType eBuildType, int nClusterSize, Vector3[] pVertexData, int nVertexDataCount, int[] pIndexData, int nIndexDataCount, Bounds bounds, ref int nClusterCount,
        ref MeshCluster[] pMeshCluster);

    [MenuItem("MeshClusterBuilder/BuildSelected")]
    public static void BuildSelected()
    {
        //Mesh meshData = AssetDatabase.LoadAssetAtPath<Mesh>("Assets/FBX/VolumeToPolygon.fbx");
        Mesh meshData = AssetDatabase.LoadAssetAtPath<Mesh>("Assets/FBX/cube.fbx");
        int[] indices = meshData.GetIndices(0);
        Debug.Log(meshData.vertices.Length + "-" + indices.Length + "-" + meshData.bounds);
        RegisterUnityLogCallback(UnityLogCallback);
        int nClusterCount = 0;
        MeshCluster[] pMeshCluster = new MeshCluster[1];
        BuildCluster(BuilderType.eUE_Metis, 128, meshData.vertices, meshData.vertices.Length, indices, indices.Length, meshData.bounds, ref nClusterCount, ref pMeshCluster);
        Debug.Log("Result --> " + nClusterCount);
    }
}
