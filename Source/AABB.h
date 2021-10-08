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
    Vector3f m_Min;
    Vector3f m_Max;
};

#endif /* AABB_h */
