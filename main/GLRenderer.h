#pragma once

#include "Math/Math.h"
#include "Core/String.h"
#include "Frame.h"

#include <GL\glew.h>


enum ShaderType
{
    Vertex,
    Fragment
};

class Shader
{
    public:
        Shader(const String _source, const ShaderType _type)
            : m_glId(0)
            , m_source(_source)
            , m_type(_type)
        {
        }

        GLuint m_glId;
        const String m_source;
        const ShaderType m_type;
};

class RenderableThing
{
    public:
        Shader m_vertexShader;
        Shader m_fragmentShader;


};

class GLRenderer
{
    private:
        Matrix4 m_projectionMatrix;

    public:

        GLRenderer();

        void init();
        void shutdown();
        void render(Frame& frame);
};
