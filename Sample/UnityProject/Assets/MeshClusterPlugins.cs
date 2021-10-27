using System;
using System.IO;
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
        public int indexCount;
        public IntPtr indexList;
    }
    
    struct MeshClusterResult
    {
        public int meshClusterCount;
        public IntPtr meshClusterList;
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
    static extern void BuildCluster(BuilderType eBuildType, int nClusterSize, Vector3[] pVertexData, int nVertexDataCount, int[] pIndexData, int nIndexDataCount, Bounds bounds, IntPtr resultPtr);

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
        //MeshCluster[] pMeshCluster = new MeshCluster[1];
        //BuildCluster(BuilderType.eUE_Metis, 128, meshData.vertices, meshData.vertices.Length, indices, indices.Length, meshData.bounds, ref nClusterCount, ref pMeshCluster);
        Debug.Log("Result --> " + nClusterCount);
    }

    [MenuItem("MeshClusterBuilder/TestSample1ForUE")]
    public static void TestSample1ForUE()
    {
        RegisterUnityLogCallback(UnityLogCallback);
        
        List<Vector3> vertices = new List<Vector3>();
        List<int> indices = new List<int>();
        
        string dataFilePath = Application.dataPath + "/TestData/ue_iron_man_data.txt";
        ReadDataFromFile(dataFilePath, vertices, indices);
        
        Bounds bounds = new Bounds();
        bounds.SetMinMax(new Vector3(-0.854617f, -3.70903397f, -0.403672993f), new Vector3(0.854617f, 3.70903397f, 0.403672993f));
        
        int meshClusterResultSize = Marshal.SizeOf<MeshClusterResult>();
        IntPtr resultPtr = Marshal.AllocHGlobal(meshClusterResultSize);
        BuildCluster(BuilderType.eUE_Metis, 128, vertices.ToArray(), vertices.Count, indices.ToArray(), indices.Count, bounds, resultPtr);
        MeshClusterResult result = Marshal.PtrToStructure<MeshClusterResult>(resultPtr);
        MeshCluster meshCluster = Marshal.PtrToStructure<MeshCluster>(result.meshClusterList);
        Debug.Log(result.meshClusterCount);
        Debug.Log(meshCluster.indexCount);
        for (int i = 0; i < meshCluster.indexCount; i++)
            Debug.Log(Marshal.PtrToStructure<UInt32>(meshCluster.indexList));
    }

    [MenuItem("MeshClusterBuilder/TestSample1ForMS")]
    public static void TestSample1ForMS()
    {
        RegisterUnityLogCallback(UnityLogCallback);

        List<Vector3> vertices = new List<Vector3>();
        List<int> indices = new List<int>();
        
        string dataFilePath = Application.dataPath + "/TestData/ms_iron_man_data.txt";
        ReadDataFromFile(dataFilePath, vertices, indices);

        Bounds bounds = new Bounds();
        bounds.SetMinMax(new Vector3(-100.00f, -100.00f, -100.00f), new Vector3(100.00f, 100.00f, 100.00f));

        int nClusterCount = 0;
        //MeshCluster[] pMeshCluster = null;
        //BuildCluster(BuilderType.eMS_Meshlet, 64, vertices.ToArray(), vertices.Count, indices.ToArray(), indices.Count, bounds, ref nClusterCount, ref pMeshCluster);
        Debug.Log("Result --> " + nClusterCount);
    }

    private static void ReadDataFromFile(string dataFilePath, List<Vector3> vertices, List<int> indices)
    {
        string[] lines = File.ReadAllLines(dataFilePath);
        for (int i = 0; i < lines.Length; i++)
        {
            string line = lines[i];
            if (line.StartsWith("vertices:"))
            {
                line = line.Replace("vertices:", "");
                string[] datas = line.Split(new char[]{','});
                Vector3 vertex = new Vector3(float.Parse(datas[0]), float.Parse(datas[1]), float.Parse(datas[2]));
                vertices.Add(vertex);
                continue;
            }

            if (line.StartsWith("indices:"))
            {
                line = line.Replace("indices:", "");
                int index = int.Parse(line);
                indices.Add(index);
            }
        }
    }
}