#pragma once

#include "Types.h"

class Vector4
{
private:
    float m_data[4];

public:
    Vector4()
        : m_data{ 0.f, 0.f, 0.f, 0.f }
    {
    }

    Vector4(const float& _x, const float& _y, const float& _z, const float& _w)
        : m_data{ _x, _y, _z, _w }
    {
    }

    const inline float& GetX() const { return m_data[0]; }
    const inline float& GetY() const { return m_data[1]; }
    const inline float& GetZ() const { return m_data[2]; }
    const inline float& GetW() const { return m_data[3]; }

    const inline float& operator[](uint32 i) const { return m_data[i]; }

    float Dot(Vector4& other) const;
};
