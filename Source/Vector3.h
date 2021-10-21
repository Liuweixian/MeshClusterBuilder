//
//  Vector3.h
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/9/30.
//

#ifndef Vector3_h
#define Vector3_h
#include <math.h>

class Vector3f
{
public:
    Vector3f() {}
    Vector3f(float inX, float inY, float inZ)  { x = inX; y = inY; z = inZ; }
    bool operator==(const Vector3f& v) const { return x == v.x && y == v.y && z == v.z; }
    Vector3f& operator+=(const Vector3f& inV) { x += inV.x; y += inV.y; z += inV.z; return *this; }
    Vector3f& operator/=(float s);
    
    float x;
    float y;
    float z;
};

inline Vector3f operator*(const Vector3f& inV, const float s) { return Vector3f(inV.x * s, inV.y * s, inV.z * s); }
inline Vector3f operator+(const Vector3f& lhs, const Vector3f& rhs) { return Vector3f(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z); }
inline Vector3f operator-(const Vector3f& lhs, const Vector3f& rhs) { return Vector3f(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z); }
inline Vector3f operator/(const Vector3f& inV, const float s) { Vector3f temp(inV); temp /= s; return temp; }

inline float Dot(const Vector3f& lhs, const Vector3f& rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }
inline Vector3f Cross(const Vector3f& lhs, const Vector3f& rhs)
{
    return Vector3f(lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x);
}
inline float Magnitude(const Vector3f& inV) {return sqrt(Dot(inV, inV)); }
inline Vector3f Normalize(const Vector3f& inV) { return inV / Magnitude(inV); }
inline Vector3f& Vector3f::operator/=(float s)
{
    x /= s;
    y /= s;
    z /= s;
    return *this;
}
inline float Distance(const Vector3f& lhs, const Vector3f& rhs)
{
    float diff_x = lhs.x - rhs.x;
    float diff_y = lhs.y - rhs.y;
    float diff_z = lhs.z - rhs.z;
    return sqrt(diff_x * diff_x + diff_y * diff_y + diff_z * diff_z);
}

#endif /* Vector3_h */
