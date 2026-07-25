#pragma once

/**
 * Math3D.h – minimal column-major 4×4 matrix library (no external deps).
 *
 * Storage: column-major  m[col*4 + row]  – matches glUniformMatrix4fv directly.
 *
 *   [0]  [4]  [8]  [12]
 *   [1]  [5]  [9]  [13]
 *   [2]  [6]  [10] [14]
 *   [3]  [7]  [11] [15]
 *
 * Translation lives in column 3: m[12], m[13], m[14].
 */

#include <cmath>
#include <cstring>

struct mat4 {
    float m[16];
    mat4() { std::memset(m, 0, sizeof(m)); }
    float& operator()(int r, int c)       { return m[c * 4 + r]; }
    float  operator()(int r, int c) const { return m[c * 4 + r]; }
};

inline mat4 mat4Identity()
{
    mat4 r;
    r(0,0) = r(1,1) = r(2,2) = r(3,3) = 1.0f;
    return r;
}

inline mat4 mat4Mul(const mat4& a, const mat4& b)
{
    mat4 r;
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row) {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k) s += a(row,k) * b(k,c);
            r(row,c) = s;
        }
    return r;
}

inline mat4 mat4Perspective(float fovYDeg, float aspect, float nearZ, float farZ)
{
    mat4 r;
    float f = 1.0f / std::tan(fovYDeg * 0.5f * (3.14159265358979f / 180.0f));
    r(0,0) = f / aspect;
    r(1,1) = f;
    r(2,2) = (farZ + nearZ) / (nearZ - farZ);
    r(3,2) = -1.0f;
    r(2,3) = (2.0f * farZ * nearZ) / (nearZ - farZ);
    return r;
}

inline mat4 mat4Translate(const mat4& in, float tx, float ty, float tz)
{
    mat4 t = mat4Identity();
    t(0,3) = tx; t(1,3) = ty; t(2,3) = tz;
    return mat4Mul(t, in);
}

inline mat4 mat4Rotate(const mat4& in, float angleDeg, float ax, float ay, float az)
{
    float rad = angleDeg * (3.14159265358979f / 180.0f);
    float c = std::cos(rad), s = std::sin(rad), oc = 1.0f - c;
    float len = std::sqrt(ax*ax + ay*ay + az*az);
    if (len < 1e-6f) return in;
    ax /= len; ay /= len; az /= len;

    mat4 r = mat4Identity();
    r(0,0) = c+ax*ax*oc;      r(0,1) = ax*ay*oc-az*s;  r(0,2) = ax*az*oc+ay*s;
    r(1,0) = ay*ax*oc+az*s;   r(1,1) = c+ay*ay*oc;     r(1,2) = ay*az*oc-ax*s;
    r(2,0) = az*ax*oc-ay*s;   r(2,1) = az*ay*oc+ax*s;  r(2,2) = c+az*az*oc;
    return mat4Mul(r, in);
}
