#include <assert.h> 
#include "Vector.h"

Vector4f::Vector4f() : _data{ 0 } {};

Vector4f::Vector4f(float x, float y, float z, float w) {
        _data[XI] = x;
        _data[YI] = y;
        _data[ZI] = z;
        _data[WI] = w;
    }

const float inline Vector4f::at(const char i) const {
    assert(i >= 0 && i < 4);
    return _data[i];
}

const bool Vector4f::isRow() const { return _isRow; }

void Vector4f::set(char i, float val) { _data[i] = val; }
void Vector4f::setX(float x) { set(XI, x); }
void Vector4f::setY(float y) { set(YI, y); }
void Vector4f::setZ(float z) { set(ZI, z); }
void Vector4f::setW(float w) { set(WI, w); }

void Vector4f::transpose() {
    this->_isRow = !this->_isRow;
}

const float Vector4f::dot(Vector4f* other) const {
    float dot = 0;

    for (char i = 0; i < 4; i++) {
        dot += this->at(i) * other->at(i);
    }

    return dot;
}
