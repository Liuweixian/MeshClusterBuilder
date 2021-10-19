using System;
using System.Collections.Generic;
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
        RegisterUnityLogCallback(UnityLogCallback);

        //Mesh meshData = AssetDatabase.LoadAssetAtPath<Mesh>("Assets/FBX/VolumeToPolygon.fbx");
        Mesh meshData = AssetDatabase.LoadAssetAtPath<Mesh>("Assets/FBX/plane.fbx");
        //Mesh meshData = AssetDatabase.LoadAssetAtPath<Mesh>("Assets/FBX/cube.fbx");
        //PrintVertices(meshData);
        //return;
        int[] indices = meshData.GetIndices(0);
        Debug.Log(meshData.vertices.Length + "-" + indices.Length + "-" + meshData.bounds);

        int nClusterCount = 0;
        MeshCluster[] pMeshCluster = new MeshCluster[1];
        BuildCluster(BuilderType.eUE_Metis, 128, meshData.vertices, meshData.vertices.Length, indices, indices.Length, meshData.bounds, ref nClusterCount, ref pMeshCluster);
        Debug.Log("Result --> " + nClusterCount);
    }

    [MenuItem("MeshClusterBuilder/TestSample1ForUE")]
    public static void TestSample1ForUE()
    {
        RegisterUnityLogCallback(UnityLogCallback);

        List<Vector3> vertices = new List<Vector3>();
        vertices.Add(new Vector3(-100.00f, -100.00f, 100.00f));
        vertices.Add(new Vector3(-100.00f, 100.00f, -100.00f));
        vertices.Add(new Vector3(-100.00f, 100.00f, 100.00f));
        vertices.Add(new Vector3(-100.00f, -100.00f, -100.00f));
        vertices.Add(new Vector3(100.00f, -100.00f, 100.00f));
        vertices.Add(new Vector3(-100.00f, -100.00f, -100.00f));
        vertices.Add(new Vector3(-100.00f, -100.00f, 100.00f));
        vertices.Add(new Vector3(100.00f, -100.00f, -100.00f));
        vertices.Add(new Vector3(100.00f, 100.00f, 100.00f));
        vertices.Add(new Vector3(100.00f, -100.00f, -100.00f));
        vertices.Add(new Vector3(100.00f, -100.00f, 100.00f));
        vertices.Add(new Vector3(100.00f, 100.00f, -100.00f));
        vertices.Add(new Vector3(-100.00f, 100.00f, 100.00f));
        vertices.Add(new Vector3(100.00f, 100.00f, -100.00f));
        vertices.Add(new Vector3(100.00f, 100.00f, 100.00f));
        vertices.Add(new Vector3(-100.00f, 100.00f, -100.00f));
        vertices.Add(new Vector3(100.00f, 100.00f, -100.00f));
        vertices.Add(new Vector3(-100.00f, 100.00f, -100.00f));
        vertices.Add(new Vector3(-100.00f, -100.00f, -100.00f));
        vertices.Add(new Vector3(100.00f, -100.00f, -100.00f));
        vertices.Add(new Vector3(-100.00f, -100.00f, 100.00f));
        vertices.Add(new Vector3(-100.00f, 100.00f, 100.00f));
        vertices.Add(new Vector3(100.00f, -100.00f, 100.00f));
        vertices.Add(new Vector3(100.00f, 100.00f, 100.00f));

        List<int> indices = new List<int>();
        indices.Add(0);
        indices.Add(1);
        indices.Add(2);
        indices.Add(1);
        indices.Add(0);
        indices.Add(3);
        indices.Add(4);
        indices.Add(5);
        indices.Add(6);
        indices.Add(5);
        indices.Add(4);
        indices.Add(7);
        indices.Add(8);
        indices.Add(9);
        indices.Add(10);
        indices.Add(9);
        indices.Add(8);
        indices.Add(11);
        indices.Add(12);
        indices.Add(13);
        indices.Add(14);
        indices.Add(13);
        indices.Add(12);
        indices.Add(15);
        indices.Add(16);
        indices.Add(17);
        indices.Add(18);
        indices.Add(16);
        indices.Add(18);
        indices.Add(19);
        indices.Add(20);
        indices.Add(21);
        indices.Add(22);
        indices.Add(22);
        indices.Add(21);
        indices.Add(23);

        Bounds bounds = new Bounds();
        bounds.SetMinMax(new Vector3(-100.00f, -100.00f, -100.00f), new Vector3(100.00f, 100.00f, 100.00f));

        int nClusterCount = 0;
        MeshCluster[] pMeshCluster = new MeshCluster[1];
        BuildCluster(BuilderType.eUE_Metis, 128, vertices.ToArray(), vertices.Count, indices.ToArray(), indices.Count, bounds, ref nClusterCount, ref pMeshCluster);
        Debug.Log("Result --> " + nClusterCount);
    }
}