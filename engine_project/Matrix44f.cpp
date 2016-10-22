#include <assert.h> 

#include "Vector.h"
#include "Matrix.h"
    

Matrix44f::Matrix44f() :
    _data{ 0 }
{
};

Matrix44f::Matrix44f(
    const float x1, const float y1, const float z1, const float w1,
    const float x2, const float y2, const float z2, const float w2,
    const float x3, const float y3, const float z3, const float w3,
    const float x4, const float y4, const float z4, const float w4
) {
    _data[X1I] = x1; _data[Y1I] = y1; _data[Z1I] = z1; _data[W1I] = w1;
    _data[X2I] = x2; _data[Y2I] = y2; _data[Z2I] = z2; _data[W2I] = w2;
    _data[X3I] = x3; _data[Y3I] = y3; _data[Z3I] = z3; _data[W3I] = w3;
    _data[X4I] = x4; _data[Y4I] = y4; _data[Z4I] = z4; _data[W4I] = w4;
}

Matrix44f Matrix44f::Identity() {
    return Matrix44f(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}

void Matrix44f::transpose() {
    const char oldColStep = this->_colStep;
    this->_colStep = this->_rowStep;
    this->_rowStep = oldColStep;
}

const float inline Matrix44f::at(const char col, const char row) const {
    return _data[col * _colStep + row * _rowStep];
}

const void inline Matrix44f::set(const char col, const char row, const float val) {
    _data[col * _colStep + row * _rowStep] = val;
}

const float* Matrix44f::data() inline const {
    return _data;
}

// TODO: test
Matrix44f Matrix44f::operator*(Matrix44f* other) {
    Matrix44f newMatrix;

    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {

            float val = 0;
            for (int k = 0; k < 4; k++) {
                val += this->at(k, row) * other->at(col, k);
            }

            newMatrix.set(col, row, val);
        }
    }

    return newMatrix;
}

// TODO: test
Vector4f Matrix44f::operator*(Vector4f* vec) {
    assert(vec->isRow());

    Vector4f newVector;

    for (int col = 0; col < 4; col++) {
        float val = 0;

        for (int k = 0; k < 4; k++) {
            val += this->at(col, k) * vec->at(k);
        }

        newVector.set(col, val);
    }

    return newVector;
}