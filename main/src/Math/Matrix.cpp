#include <cmath>

#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Math/Constants.h"

const Matrix4 Matrix4::s_identity = Matrix4(
                                        Vector4(1.f, 0.f, 0.f, 0.f),
                                        Vector4(0.f, 1.f, 0.f, 0.f),
                                        Vector4(0.f, 0.f, 1.f, 0.f),
                                        Vector4(0.f, 0.f, 0.f, 1.f)
                                    );
