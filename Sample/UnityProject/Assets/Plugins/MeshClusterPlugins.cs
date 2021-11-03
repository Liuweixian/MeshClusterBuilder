using System;
using System.IO;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEditor;
using UnityEngine;
using Random = System.Random;

public class MeshClusterPlugins
{
    enum BuilderType
    {
        eUE_Metis,
        eMS_Meshlet
    };

    struct MeshClusterForPlugin
    {
        public int IndexCount;
        public IntPtr IndicesIntPtr;
    }
    
    struct MeshClusterResultForPlugin
    {
        public int MeshClusterCount;
        public IntPtr MeshClustersIntPtr;
    }

    struct MeshCluster
    {
        public int IndexCount;
        public List<UInt32> Indices;
    }

    struct MeshClusterResult
    {
        public int MeshClusterCount;
        public List<MeshCluster> MeshClusters;
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
    
    [DllImport("MeshClusterBuilder", CallingConvention = CallingConvention.Cdecl)]
    static extern void ReleaseCulsterResult(IntPtr resultPtr);

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

    private static IntPtr CreatePluginResult()
    {
        int meshClusterResultSize = Marshal.SizeOf<MeshClusterResultForPlugin>();
        return Marshal.AllocHGlobal(meshClusterResultSize);
    }

    private static MeshClusterResult ParsingResult(IntPtr resultIntPtr)
    {
        MeshClusterResultForPlugin resultForPlugin = Marshal.PtrToStructure<MeshClusterResultForPlugin>(resultIntPtr);
        MeshClusterResult meshClusterResult = new MeshClusterResult();
        meshClusterResult.MeshClusterCount = resultForPlugin.MeshClusterCount;
        meshClusterResult.MeshClusters = new List<MeshCluster>(meshClusterResult.MeshClusterCount);
        int sizeOfMeshClusterForPlugin = Marshal.SizeOf<MeshClusterForPlugin>();
        int sizeOfUInt32 = Marshal.SizeOf<UInt32>();
        for (int i = 0; i < meshClusterResult.MeshClusterCount; i++)
        {
            MeshClusterForPlugin meshClusterForPlugin = Marshal.PtrToStructure<MeshClusterForPlugin>(resultForPlugin.MeshClustersIntPtr + i * sizeOfMeshClusterForPlugin);
            MeshCluster meshCluster = new MeshCluster();
            meshCluster.IndexCount = meshClusterForPlugin.IndexCount;
            meshCluster.Indices = new List<UInt32>(meshCluster.IndexCount);
            for (int j = 0; j < meshCluster.IndexCount; j++)
            {
                UInt32 index = Marshal.PtrToStructure<UInt32>(meshClusterForPlugin.IndicesIntPtr + j * sizeOfUInt32);
                meshCluster.Indices.Add(index);
            }
            meshClusterResult.MeshClusters.Add(meshCluster);
        }

        return meshClusterResult;
    }

    private static void ReleasePluginResult(IntPtr resultIntPtr)
    {
        ReleaseCulsterResult(resultIntPtr);
        Marshal.FreeHGlobal(resultIntPtr);
    }

    [MenuItem("MeshClusterBuilder/TestSample1ForUE")]
    private static void TestSample1ForUE()
    {
        RegisterUnityLogCallback(UnityLogCallback);
        
        List<Vector3> vertices = new List<Vector3>();
        List<int> indices = new List<int>();
        
        string dataFilePath = Application.dataPath + "/TestData/ue_iron_man_data.txt";
        ReadDataFromFile(dataFilePath, vertices, indices);
        
        Bounds bounds = new Bounds();
        bounds.SetMinMax(new Vector3(-0.854617f, -3.70903397f, -0.403672993f), new Vector3(0.854617f, 3.70903397f, 0.403672993f));

        IntPtr resultPtr = CreatePluginResult();
        BuildCluster(BuilderType.eUE_Metis, 128, vertices.ToArray(), vertices.Count, indices.ToArray(), indices.Count, bounds, resultPtr);
        MeshClusterResult meshClusterResult = ParsingResult(resultPtr);
        ReleasePluginResult(resultPtr);
        CreateDebugMesh(vertices, indices, meshClusterResult, "ue_iron_man_data");
    }

    [MenuItem("MeshClusterBuilder/TestSample1ForMS")]
    private static void TestSample1ForMS()
    {
        RegisterUnityLogCallback(UnityLogCallback);

        List<Vector3> vertices = new List<Vector3>();
        List<int> indices = new List<int>();
        
        string dataFilePath = Application.dataPath + "/TestData/ms_iron_man_data.txt";
        ReadDataFromFile(dataFilePath, vertices, indices);

        Bounds bounds = new Bounds();
        bounds.SetMinMax(new Vector3(-100.00f, -100.00f, -100.00f), new Vector3(100.00f, 100.00f, 100.00f));

        IntPtr resultPtr = CreatePluginResult();
        BuildCluster(BuilderType.eMS_Meshlet, 64, vertices.ToArray(), vertices.Count, indices.ToArray(), indices.Count, bounds, resultPtr);
        MeshClusterResult meshClusterResult = ParsingResult(resultPtr);
        ReleasePluginResult(resultPtr);
        CreateDebugMesh(vertices, indices, meshClusterResult, "ms_iron_man_data");
    }

    private static void CreateDebugMesh(List<Vector3> vertices, List<int> indices, MeshClusterResult meshClusterResult, string assetName)
    {
        string outputDir = "Assets/Output-" + assetName + "/";
        if (!Directory.Exists(outputDir))
            Directory.CreateDirectory(outputDir);

        GameObject root = new GameObject(assetName);
        Material material = AssetDatabase.LoadAssetAtPath<Material>("Assets/Plugins/MeshClusterDebugMat.mat");
        
        Random random = new Random();
        for (int i = 0; i < meshClusterResult.MeshClusterCount; i++)
        {
            Color curClusterColor = new Color(random.Next(0, 255) / 255.0f, random.Next(0, 255) / 255.0f, random.Next(0, 255) / 255.0f);
            Dictionary<int, int> indexMapping = new Dictionary<int, int>();
            List<Vector3> newVertices = new List<Vector3>();
            List<int> newIndices = new List<int>();
            List<Color> newColors = new List<Color>();
            
            MeshCluster meshCluster = meshClusterResult.MeshClusters[i];
            for (int j = 0; j < meshCluster.IndexCount; j++)
            {
                int vertIndex = (int)meshCluster.Indices[j];
                if (indexMapping.ContainsKey(vertIndex))
                {
                    newIndices.Add(indexMapping[vertIndex]);
                    continue;
                }

                int curIndex = newVertices.Count;
                newVertices.Add(vertices[vertIndex]);
                newIndices.Add(curIndex);
                newColors.Add(curClusterColor);
                indexMapping[vertIndex] = curIndex;
            }
            
            Mesh mesh = new Mesh();
            mesh.SetVertices(newVertices);
            mesh.SetIndices(newIndices, MeshTopology.Triangles, 0, true);
            mesh.SetColors(newColors);
            string clusterName = "cluster-" + i;
            AssetDatabase.CreateAsset(mesh, outputDir + clusterName + ".asset");

            GameObject cluster = new GameObject(clusterName);
            cluster.transform.parent = root.transform;
            MeshFilter meshFilter = cluster.AddComponent<MeshFilter>();
            meshFilter.sharedMesh = mesh;
            MeshRenderer meshRenderer = cluster.AddComponent<MeshRenderer>();
            meshRenderer.sharedMaterial = material;
        }
        AssetDatabase.Refresh();
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