#include "Types.h"
#include "Math/Vector.h"


float Vector4::Dot(Vector4& other) const
{
    float dot = 0.f;
    for (uint32 i = 0; i < 4; ++i)
    {
        dot += other[i] * (*this)[i];
    }

    return dot;
}
