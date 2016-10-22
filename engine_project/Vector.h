#pragma once

const char XI = 0;
const char YI = 1;
const char ZI = 2;
const char WI = 3;

class Vector4f {
public:
    Vector4f();
    Vector4f(float x, float y, float z, float w);

    const float Vector4f::at(const char i) const;
    const bool Vector4f::isRow() const;
    const float Vector4f::dot(Vector4f* other) const;

    void Vector4f::set(char i, float val);
    void Vector4f::setX(float x);
    void Vector4f::setY(float y);
    void Vector4f::setZ(float z);
    void Vector4f::setW(float w);
    void Vector4f::transpose();

private:
    float _data[4];
    bool _isRow = true;
};