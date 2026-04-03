#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Type/TraitMacros.h>

KAI_BEGIN

struct Vector2 {
    Vector2() : x(0), y(0) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}
    float x, y;
};

struct Vector3 {
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    float x, y, z;
};

struct Vector4 {
    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(float _x, float _y, float _z, float _w)
        : x(_x), y(_y), z(_z), w(_w) {}
    float x, y, z, w;
    Vector4 &operator*=(float a) {
        x *= a;
        y *= a;
        z *= a;
        w *= a;
        return *this;
    }
    Vector4 &operator/=(float a) {
        x /= a;
        y /= a;
        z /= a;
        w /= a;
        return *this;
    }
    Vector4 &operator+=(Vector4 const &R) {
        x += R.x;
        y += R.y;
        z += R.z;
        w += R.z;
        return *this;
    }
    Vector4 &operator-=(Vector4 const &R) {
        x -= R.x;
        y -= R.y;
        z -= R.z;
        w -= R.w;
        return *this;
    }
    friend Vector4 operator*(Vector4 const &A, float B) {
        Vector4 R(A);
        R *= B;
        return R;
    }
    friend Vector4 operator/(Vector4 const &A, float B) {
        Vector4 R(A);
        R /= B;
        return R;
    }
    friend Vector4 operator+(Vector4 const &A, Vector4 const &B) {
        Vector4 R(A);
        R += B;
        return R;
    }
    friend Vector4 operator-(Vector4 const &A, Vector4 const &B) {
        Vector4 R(A);
        R -= B;
        return R;
    }
};

StringStream &operator<<(StringStream &, Vector2 const &);
StringStream &operator>>(StringStream &, Vector2 &);
BinaryStream &operator<<(BinaryStream &, Vector2 const &);
BinaryStream &operator>>(BinaryStream &, Vector2 &);

StringStream &operator<<(StringStream &, Vector3 const &);
StringStream &operator>>(StringStream &, Vector3 &);
BinaryStream &operator<<(BinaryStream &, Vector3 const &);
BinaryStream &operator>>(BinaryStream &, Vector3 &);

StringStream &operator<<(StringStream &, Vector4 const &);
StringStream &operator>>(StringStream &, Vector4 &);
BinaryStream &operator<<(BinaryStream &, Vector4 const &);
BinaryStream &operator>>(BinaryStream &, Vector4 &);

KAI_TYPE_TRAITS(Vector2, Number::Vector2,
                Properties::Streaming | Properties::Assign
                //| Properties::Plus
                //| Properties::Minus
                //| Properties::Absolute
);

KAI_TYPE_TRAITS(Vector3, Number::Vector3,
                Properties::Streaming | Properties::Assign
                //| Properties::Plus
                //| Properties::Minus
                //| Properties::Absolute
);

KAI_TYPE_TRAITS(Vector4, Number::Vector4,
                Properties::Streaming | Properties::Plus
                //| Properties::Minus
                //| Properties::Assign
                //| Properties::Absolute
);

KAI_END
