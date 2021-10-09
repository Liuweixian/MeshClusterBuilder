//
//  DisjointSet.h
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/8.
//

#ifndef DisjointSet_h
#define DisjointSet_h
#include <vector>

class DisjointSet
{
public:
    DisjointSet() {}
    DisjointSet(const UInt32 size);
    void    Init(UInt32 size);
    void    Reset();
    void    AddDefaulted(UInt32 num = 1);
    void    Union(UInt32 x, UInt32 y);
    void    UnionSequential(UInt32 x, UInt32 y);
    UInt32    Find(UInt32 i);
    UInt32    operator[](UInt32 i) const { return m_parents[i]; }
private:
    std::vector<UInt32> m_parents;
};
inline DisjointSet::DisjointSet(const UInt32 size)
{
    Init(size);
}
inline void DisjointSet::Init(UInt32 size)
{
    m_parents.reserve(size);
    for (UInt32 i = 0; i < size; i++)
    {
        m_parents[i] = i;
    }
}
inline void DisjointSet::Reset()
{
    m_parents.clear();
}

inline void DisjointSet::AddDefaulted(UInt32 num)
{
    /*UInt32 start = m_parents.size();
    dynamic_array<UInt32>::iterator iter = m_parents.insert(m_parents.end(), num);
    for (; iter != m_parents.end(); ++iter)
    {
        *iter = start++;
    }*/
}

// Union with splicing
inline void DisjointSet::Union(UInt32 x, UInt32 y)
{
    UInt32 px = m_parents[x];
    UInt32 py = m_parents[y];
    while (px != py)
    {
        // Pick larger
        if (px < py)
        {
            m_parents[x] = py;
            if (x == px)
            {
                return;
            }
            x = px;
            px = m_parents[x];
        }
        else
        {
            m_parents[y] = px;
            if (y == py)
            {
                return;
            }
            y = py;
            py = m_parents[y];
        }
    }
}

// Optimized version of Union when iterating for( x : 0 to N ) unioning x with lower indexes.
// Neither x nor y can have already been unioned with an index > x.
inline void DisjointSet::UnionSequential(UInt32 x, UInt32 y)
{
    assert(x >= y);
    assert(x == m_parents[x]);
    UInt32 px = x;
    UInt32 py = m_parents[y];
    while (px != py)
    {
        m_parents[y] = px;
        if (y == py)
        {
            return;
        }
        y = py;
        py = m_parents[y];
    }
}
// Find with path compression
inline UInt32 DisjointSet::Find(UInt32 i)
{
    // Find root
    UInt32 Start = i;
    UInt32 Root = m_parents[i];
    while (Root != i)
    {
        i = Root;
        Root = m_parents[i];
    }
    // Point all nodes on path to root
    i = Start;
    UInt32 Parent = m_parents[i];
    while (Parent != Root)
    {
        m_parents[i] = Root;
        i = Parent;
        Parent = m_parents[i];
    }
    return Root;
 
}

#endif /* DisjointSet_h */
