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


Matrix4 Matrix4::CreateProjectionMatrix(const float fov, const float farPlane, const float nearPlane)
{
    // The default camera looks down the positive z-axis.

    // There are a few goals of the projection:
    //   1. Map a point from 3d space to a 3d point in a unit cube.
    //   2. Scale z values between -1 -> 1 based on near and far clipping plane.
    //   3. Scale x and y values between -1 -> 1 (NDC coords) based on fov

    // zScale and fovScale are responsible for scaling z and x/y coordinates (goals 2. and 3.)

    // Why the 1.0f in third column's w value?:
    //      "In OpenGL, perspective division happens automatically after the vertex shader runs on each vertex.
    //      This is one reason why gl_Position, the main output of the vertex shader, is a 4D vector, not a 3D vector."
    //
    //      Open GL will automatically normalize your 4D "homogeneous" vector based on its W value when converting it to 3D.
    //      i.e. (x,y,z,w) becomes (x/w, y/w, z/w)
    //
    //      Because of this, if we set w to z, then we get the projection!
    //      i.e. (x,y,z,z) becomes (x/z, y/z, z/z) = (x/z, y/z, 1).

    // zTransform moves the point back so it's within -1 -> 1 (instead of 0 -> 1)


    const float zScale = farPlane / (farPlane - nearPlane); // negative since camera looks down -z axis
    const float zTransform = -(farPlane * nearPlane) / (farPlane - nearPlane);

    const float fovScale = 1.0f / tan((fov / 2.0f) * (PI_F / 180.0f));

    return Matrix4(
               Vector4(fovScale, 0.0f,     0.0f,     0.0f),
               Vector4(0.0f,     fovScale, 0.0f,     0.0f),
               Vector4(0.0f,     0.0f,     zScale,   zTransform),
               Vector4(0.0f,     0.0f,     1.0f,    0.0f)
           );
}
