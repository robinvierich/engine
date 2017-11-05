#pragma once

#include "Vector.h"

class Matrix4
{
private:
    Vector4 m_columns[4];

    const static Matrix4 s_identity;

public:

    static const Matrix4& Identity() { return s_identity; }

    Matrix4()
        : m_columns{}
    {
    }

    Matrix4(Vector4& col0, Vector4& col1, Vector4& col2, Vector4& col3)
        : m_columns{ col0, col1, col2, col3 }
    {
    }


    const inline float& operator[](uint32 i) const
    {
        return m_columns[i / 4][i % 4];
    }
};
