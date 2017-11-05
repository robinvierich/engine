#pragma once

#include "Vector.h"

// COLUMN-MAJOR ORDER! (match OpenGL)
const char X1I = 0; const char Y1I = 4; const char Z1I = 8;  const char W1I = 12;
const char X2I = 1; const char Y2I = 5; const char Z2I = 9;  const char W2I = 13;
const char X3I = 2; const char Y3I = 6; const char Z3I = 10; const char W3I = 14;
const char X4I = 3; const char Y4I = 7; const char Z4I = 11; const char W4I = 15;
  
// Rows represent individual vectors
// Columns are elements within those vectors
class Matrix44f {
public:
    Matrix44f();
    Matrix44f(
        const float x1, const float x2, const float x3, const float x4,
        const float y1, const float y2, const float y3, const float y4,
        const float z1, const float z2, const float z3, const float z4,
        const float w1, const float w2, const float w3, const float w4
    );

    static Matrix44f Identity();

    const float inline Matrix44f::at(const char col, const char row) const;
    const void inline Matrix44f::set(const char col, const char row, const float val);
    const float* Matrix44f::data() inline const;

    Matrix44f Matrix44f::operator*(Matrix44f* other);
    Vector4f Matrix44f::operator*(Vector4f* other);

    void Matrix44f::transpose();

    /*float Matrix44::X1() const; float Matrix44::Y1() const; float Matrix44::Z1() const; float Matrix44::W1() const;
    float Matrix44::X2() const; float Matrix44::Y2() const; float Matrix44::Z2() const; float Matrix44::W2() const;
    float Matrix44::X3() const; float Matrix44::Y3() const; float Matrix44::Z3() const; float Matrix44::W3() const;
    float Matrix44::X4() const; float Matrix44::Y4() const; float Matrix44::Z4() const; float Matrix44::W4() const;


    void Matrix44::X1(); void Matrix44::Y1(); void Matrix44::Z1(); void Matrix44::W1();
    void Matrix44::X2(); void Matrix44::Y2(); void Matrix44::Z2(); void Matrix44::W2();
    void Matrix44::X3(); void Matrix44::Y3(); void Matrix44::Z3(); void Matrix44::W3();
    void Matrix44::X4(); void Matrix44::Y4(); void Matrix44::Z4(); void Matrix44::W4();*/

private:
    float _data[4 * 4];
    char _colStep = 4;
    char _rowStep = 1;
};