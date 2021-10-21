//
//  Vector4.h
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/21.
//

#ifndef Vector4_h
#define Vector4_h

class Vector4f
{
public:
    Vector4f() {}
    Vector4f(const Vector4f& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    Vector4f(float inX, float inY, float inZ, float inW) : x(inX), y(inY), z(inZ), w(inW) {}
    explicit Vector4f(const Vector3f& v, float inW) : x(v.x), y(v.y), z(v.z), w(inW) {}
    float x;
    float y;
    float z;
    float w;
};
inline Vector4f operator/(const Vector4f& inV, const float s) { return Vector4f(inV.x / s, inV.y / s, inV.z / s, inV.w / s); }
inline float Dot(const Vector4f& lhs, const Vector4f& rhs)             { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w; }
inline float Magnitude(const Vector4f& inV)                            { return sqrt(Dot(inV, inV)); }
inline Vector4f Normalize(const Vector4f& inV) { return inV / Magnitude(inV); }

#endif /* Vector4_h */
