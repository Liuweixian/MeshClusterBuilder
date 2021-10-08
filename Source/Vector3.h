//
//  Vector3.h
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/9/30.
//

#ifndef Vector3_h
#define Vector3_h

class Vector3f
{
public:
    float x;
    float y;
    float z;
    
    bool operator==(const Vector3f& v) const { return x == v.x && y == v.y && z == v.z; }
};

#endif /* Vector3_h */
