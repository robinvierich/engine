#include "Matrix.h"
#include "Vector.h"

const static Matrix4 s_identity = Matrix4(
    Vector4(1.f, 0.f, 0.f, 0.f),
    Vector4(0.f, 1.f, 0.f, 0.f),
    Vector4(0.f, 0.f, 1.f, 0.f),
    Vector4(0.f, 0.f, 0.f, 1.f)
);
