//
//  AABB.h
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/9/30.
//

#ifndef AABB_h
#define AABB_h

#include "Vector3.h"

class AABB
{
public:
    Vector3f m_Center;
    Vector3f m_Extent;
};

class MinMaxAABB
{
public:
    explicit MinMaxAABB(const AABB& aabb) { FromAABB(aabb); }
    Vector3f m_Min;
    Vector3f m_Max;
    const Vector3f& GetMin() const { return m_Min; }
    const Vector3f& GetMax() const { return m_Max; }
private:
    void FromAABB(const AABB& inAABB);
};

inline void MinMaxAABB::FromAABB(const AABB& inAABB)
{
    m_Min = inAABB.m_Center - inAABB.m_Extent;
    m_Max = inAABB.m_Center + inAABB.m_Extent;
}

#endif /* AABB_h */
