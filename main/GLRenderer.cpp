#include "Math/Math.h"
#include "GLRenderer.h"

#include <GL\glew.h>


GLRenderer::GLRenderer()
    : m_projectionMatrix(Matrix4::CreateProjectionMatrix(60, 1000, 1))
{
}

void GLRenderer::init()
{
    static const GLuint POSITION_ATTRIB_INDEX = 0;
    static const GLuint UV_ATTRIB_INDEX = 1;

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // create and link shaders
    vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertex_shader, NULL);
    glCompileShader(vShader);

    fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragment_shader, NULL);
    glCompileShader(fShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, fShader);
    glAttachShader(shaderProgram, vShader);
    glLinkProgram(shaderProgram);

    if (glGetError() != GL_NO_ERROR)
    {
        int a = 1;
        a += 1;
    }

    // check for shader issues
    GLint status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        std::string msg("Program linking failure: ");

        GLint infoLogLength;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
        char* strInfoLog = new char[infoLogLength + 1];
        glGetProgramInfoLog(shaderProgram, infoLogLength, NULL, strInfoLog);
        msg += strInfoLog;
        delete[] strInfoLog;

        glDeleteProgram(shaderProgram); shaderProgram = 0;
        throw std::runtime_error(msg);
    }

    glUseProgram(shaderProgram);

    // Create uniforms for model/camera/projection matrices
    GLuint modelMatrixUniformLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
    GLuint cameraMatrixUniformLocation = glGetUniformLocation(shaderProgram, "cameraMatrix");
    GLuint projectionMatrixUniformLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");

    glUniformMatrix4fv(modelMatrixUniformLocation, 1, GL_FALSE, &modelToWorld[0]);
    glUniformMatrix4fv(cameraMatrixUniformLocation, 1, GL_FALSE, &cameraToWorld[0]);
    glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &cameraToScreenProj[0]);
}


void GLRenderer::shutdown()
{
}

void GLRenderer::render(Frame& frame)
{
}