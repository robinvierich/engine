#include <windows.h>
#include <tchar.h>

#include <iostream>
#include <sstream>

#include <GL\glew.h>
#include <GL\wglew.h>

#pragma comment(lib, "glew32.lib")

#define DBOUT( s )            \
{                             \
   std::ostringstream os_;    \
   os_ << s;                   \
   OutputDebugString( os_.str().c_str() );  \
}


const char* vertex_shader =
"#version 400\n"
"in vec3 vp;"
"void main () {"
"  gl_Position = vec4 (vp, 1.0);"
"}";

const char* fragment_shader =
"#version 400\n"
"out vec4 frag_colour;"
"void main () {"
"  frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
"}";

GLuint vbo = 0; // Vertex buffer object. Stores some vertex data. Can be position, normal, UV, ... In this case, position.
GLuint vao = 0; // Vertex attribute object. Stores a bunch of VBOs together in the same object (for efficiency)

GLuint vShader = 0;
GLuint fShader = 0;
GLuint shaderProgram = 0;

float points[] = { // some 3d points to draw
    0.0f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.0f, 
    0.5f, 0.0f, 0.0f,
};

void setupScene() {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    glGenVertexArrays(1, &vao); // Generate '1' vertex array for the VAO and save the new, unique name (id) in 'vao'
    glBindVertexArray(vao); // make this vertex array active (set a global variable in opengl state machine which is used by proceeding functions - ex. vertex_array = vao)

    glGenBuffers(1, &vbo); // create (1) buffer and save the 'name' in (vbo)
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // make this array buffer active (global variable in opengl state maching - ex. array_buffer = vbo
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);
    // allocate (9) (float)s of memory for the buffer, initialize with (points), and state that the data will NOT be modified (GL_STATIC_DRAW)

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        // in the active vertex attribute pointer (vao currently), define the data structure
        // At index (0), the attribute consists of (3) (GL_FLOAT)s that are NOT normalized (GL_FALSE). 
        // Between each attribute - a vec3 representing position - there are (0) bytes, AND the array starts at index 0 (NULL??). 
        //    Ex: data for this attribute looks like: [(NO OFFSET HERE) x0, y0, z0, (NO STRIDE HERE) x1, y1, z1, ...]

    // In general, VAOs look something like this (we just set up the 1st VAO index -- a position array):
    // VAO = [
    //   [ x0, y0, z0, x1, y1, z1, ...], - positions
    //   [ nx0, ny0, nz0, nx1, ny1, nz1, ...], - Normals
    //   ...
    // ]

    glEnableVertexAttribArray(0); // enable the attribute at index 0 within the vao vertex array (position in this case). If not enabled, will not be used when rendering during draw calls
                                  //glBindBuffer(GL_ARRAY_BUFFER, vbo); // make the vbo active


    
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
}


int width, height = 0;
HDC hdc = NULL; // device context handle
HGLRC glcxt = NULL; // OpenGL rendering context handle

void renderScene() {
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear the current color buffer (default framebuffer), depth buffer, and stencil buffer

    glUseProgram(shaderProgram); // set our shader program to be the current shaders

    glBindVertexArray(vao); // set (vao) to be the current vertex attribute object

    glDrawArrays(GL_TRIANGLES, 0, 3); 
        // Draw (3) (GL_TRIANGLES). Get the position data from the currently bound vertex array (vao), and start reading at index (0)
        //   What happens if we have normals as well??

    SwapBuffers(hdc); // windows command
}

void resizeWindow(int w, int h) {
    width = w;
    height = h;
}

bool initOpenGL(HWND hWnd) {
    hdc = GetDC(hWnd);

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR)); // is this necessary? Maybe I'm not updating all the fields of a PFD.
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW; // enable features
    pfd.iPixelType = PFD_TYPE_RGBA; // use RGBA pixels
    pfd.cColorBits = 32; // 32 bits of colour information (I assume this would be 24 if using RGB pixels?)
    pfd.cDepthBits = 32; // 32 bits of depth info (higher -> more depth levels / resolution)
    pfd.iLayerType = PFD_MAIN_PLANE; // Layer type? unsure.

    int pixelFormatIdx = ChoosePixelFormat(hdc, &pfd); // weird. Given the params in pfd, get an actual pixel format back that is the closest match to pfd based on hardware (hdc)
    if (pixelFormatIdx == 0) { return false; } // failed to find anything valid?

    BOOL didSetPixelFormat = SetPixelFormat(hdc, pixelFormatIdx, &pfd); // set the pixel format of hdc to pixelFormatIdx. pfd is passed in simply to log the logical pixel format requested
    if (!didSetPixelFormat) { return false; } // failed to set pixel format

    HGLRC tempOpenGLContext = wglCreateContext(hdc); // create opengl 2.1 context for hdc
    wglMakeCurrent(hdc, tempOpenGLContext); // make this context current/active

    GLenum error = glewInit(); // init glew so we can extend the opengl 2 context with features from 3 and 4
    if (error != GLEW_OK) { return false; }

    int glew_attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4, // Set the MAJOR version of OpenGL to 4  
        WGL_CONTEXT_MINOR_VERSION_ARB, 5, // Set the MINOR version of OpenGL to 5
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, // Set our OpenGL context to be forward compatible
        0 // end
    };

    if (wglewIsSupported("WGL_ARB_create_context") == 1) { // If the OpenGL 4.x context creation extension is available
        glcxt = wglCreateContextAttribsARB(hdc, NULL, glew_attributes); // Create and OpenGL 4.x context based on the given attributes 
        wglMakeCurrent(NULL, NULL); // Remove the temporary context from being active
        wglDeleteContext(tempOpenGLContext); // Delete the temporary OpenGL 2.1 context  
        wglMakeCurrent(hdc, glcxt); // Make our OpenGL 4.x context current on hdc
    } else {
        glcxt = tempOpenGLContext; // if this fails, use default OpenGL 2.1 context
    }

    int glVersion[2] = { -1, -1 }; // default version values
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]); // use GLEW to get the Major version of OpenGL we're using
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]); // use GLEW to get the Minor version of OpenGL we're using

    DBOUT("Using OpenGL " << glVersion[0] << "." << glVersion[1] << std::endl);

    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_SIZE: // If our window is resizing  
        {
            // resize our opengl context
            resizeWindow(LOWORD(lParam), HIWORD(lParam));
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {

    static TCHAR szWindowClass[] = _T("win32app");
    static TCHAR szTitle[] = _T("OpenGL app");

    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szWindowClass;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("ERROR"),
            NULL);

        return 1;
    }

    HWND hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        400, 225,
        NULL,
        NULL,
        hInstance,
        NULL
        );
    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("ERROR"),
            NULL);

        return 1;
    }

    initOpenGL(hWnd);
    setupScene();

    ShowWindow(hWnd, nCmdShow);
    
    resizeWindow(400, 225);

    UpdateWindow(hWnd);

    MSG msg;
    bool running = true; // keep track of our own 'running' var
    while (running) { 
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { // if there is a message, and handle it (why don't we need to consume it?! -- i.e. GetMessage)
            if (msg.message == WM_QUIT) {
                running = false;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else { // if no message, render!
            renderScene();
        }
    }

    return (int)msg.wParam;
}
