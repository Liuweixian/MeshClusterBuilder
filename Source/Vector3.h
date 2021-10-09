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
    Vector3f() {}
    Vector3f(float inX, float inY, float inZ)  { x = inX; y = inY; z = inZ; }
    bool operator==(const Vector3f& v) const { return x == v.x && y == v.y && z == v.z; }
    Vector3f& operator+=(const Vector3f& inV) { x += inV.x; y += inV.y; z += inV.z; return *this; }
    float x;
    float y;
    float z;
};

inline Vector3f operator*(const Vector3f& inV, const float s) { return Vector3f(inV.x * s, inV.y * s, inV.z * s); }
inline Vector3f operator+(const Vector3f& lhs, const Vector3f& rhs) { return Vector3f(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z); }
inline Vector3f operator-(const Vector3f& lhs, const Vector3f& rhs) { return Vector3f(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z); }

inline float Dot(const Vector3f& lhs, const Vector3f& rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }

#endif /* Vector3_h */
