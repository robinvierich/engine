#include <windows.h>
#include <tchar.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <GL\glew.h>
#include <GL\wglew.h>

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#pragma comment(lib, "glew32.lib")

#define DBOUT( s )            \
{                             \
   std::ostringstream os_;    \
   os_ << s;                   \
   OutputDebugString( os_.str().c_str() );  \
}

// Pulled from stackoverflow (for SHAME!!) with some modifications
void loadBmp(const char* filepath, std::vector<unsigned char>* outPixels, int* outWidth, int* outHeight)
{
    std::ifstream hFile(filepath, std::ios::in | std::ios::binary);
    if (!hFile.is_open()) throw std::invalid_argument("Error: File Not Found.");

    hFile.seekg(0, std::ios::end);
    int length = hFile.tellg();
    hFile.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> fileInfo(length);
    hFile.read(reinterpret_cast<char*>(fileInfo.data()), 54);

    if (fileInfo[0] != 'B' && fileInfo[1] != 'M')
    {
        hFile.close();
        throw std::invalid_argument("Error: Invalid File Format. Bitmap Required.");
    }

    if (fileInfo[28] != 24 && fileInfo[28] != 32)
    {
        hFile.close();
        throw std::invalid_argument("Error: Invalid File Format. 24 or 32 bit Image Required.");
    }

    short bitsPerPixel = fileInfo[28];
    *outWidth = fileInfo[18] + (fileInfo[19] << 8);
    *outHeight = fileInfo[22] + (fileInfo[23] << 8);
    std::uint32_t pixelsOffset = fileInfo[10] + (fileInfo[11] << 8);
    std::uint32_t size = ((*outWidth * bitsPerPixel + 31) / 32) * 4 * (*outHeight);
    outPixels->resize(size);

    hFile.seekg(pixelsOffset, std::ios::beg);
    hFile.read(reinterpret_cast<char*>(outPixels->data()), size);
    hFile.close();
}


const char* vertex_shader =
"#version 400\n"
"in vec3 vp;"
"in vec2 vertTexCoord;"
"out vec2 fragTexCoord;"
"void main () {"
"  fragTexCoord = vertTexCoord;"
"  gl_Position = vec4 (vp, 1.0);"
"}";

const char* fragment_shader =
"#version 400\n"
"uniform sampler2D tex;" //this is the texture
"in vec2 fragTexCoord;" //this is the texture coord
"out vec4 frag_colour;"
"void main () {"
"   frag_colour = texture(tex, fragTexCoord);"
//"  frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
"}";

GLuint vbo = 0; // Vertex buffer object. Stores some vertex data. Can be position, normal, UV, ... In this case, position.
GLuint vao = 0; // Vertex attribute object. Stores a bunch of VBOs together in the same object (for efficiency)

GLuint vShader = 0;
GLuint fShader = 0;
GLuint shaderProgram = 0;

float points[] = { // some points to draw
//   x     y     z       u     v
    0.0f, 0.0f, -0.1f,   0.0f, 0.0f,
    0.5f, 0.5f, -0.1f,   1.0f, 1.0f,
    0.5f, 0.0f, -0.1f,   1.0f, 0.0f,
};

GLuint mappedtexture;

void setupScene() {
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

    if (glGetError() != GL_NO_ERROR) {
        int a = 1;
        a += 1;
    }

    // check for shader issues
    GLint status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
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


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// LOAD A TEXTURE
    int width = 256, height = 256;//0;
    std::vector<unsigned char> pixels;
    loadBmp("F:\\code\\engine_project\\Debug\\smiley.bmp", &pixels, &width, &height);

    glGenTextures(1, &mappedtexture);

    glBindTexture(GL_TEXTURE_2D, mappedtexture);
    // clear the texture (image of 0s)

    // super haxx on size here
    //char pixelData[256 * 256 * 3];

    unsigned char* pixelData = pixels.data();

    //for (int i = 0; i < (width * height) * 3; i += 3) {
    //    pixelData[i + 0] = 0xFF; // R
    //    pixelData[i + 1] = 0xFF; // G
    //    pixelData[i + 2] = 0xFF; // B
    //}

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB4, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);//pixels.data());

                                                                                                   // filter image
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint uniformLocation = glGetUniformLocation(shaderProgram, "tex");
    glUniform1i(uniformLocation, 3);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create vao + vbos for position and UV data
    GLuint vpLocation = glGetAttribLocation(shaderProgram, "vp");
    GLuint vertTexCoordLocation = glGetAttribLocation(shaderProgram, "vertTexCoord");

    //glBindAttribLocation(shaderProgram, POSITION_ATTRIB_INDEX, "vp");
    //glBindAttribLocation(shaderProgram, UV_ATTRIB_INDEX, "vertTexCoord");

    glGenVertexArrays(1, &vao); // Generate '1' vertex array for the VAO and save the new, unique name (id) in 'vao'
    glBindVertexArray(vao); // make this vertex array active (set a global variable in opengl state machine which is used by proceeding functions - ex. vertex_array = vao)

    glGenBuffers(1, &vbo); // create (1) buffer and save the 'name' in (vbo)
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // make this array buffer active (global variable in opengl state maching - ex. array_buffer = vbo
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
    // allocate (9) (float)s of memory for the buffer, initialize with (points), and state that the data will NOT be modified (GL_STATIC_DRAW)

    glEnableVertexAttribArray(vpLocation); // enable the attribute at index 0 within the vao vertex array (position in this case). If not enabled, will not be used when rendering during draw calls
                                  //glBindBuffer(GL_ARRAY_BUFFER, vbo); // make the vbo active

    glVertexAttribPointer(vpLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), NULL);
        // this is used to bind vertex attributes to the shader 
        // in the active vertex attribute object (vao currently), define the data structure
        // At index (0), the attribute consists of (3) (GL_FLOAT)s that are NOT normalized (GL_FALSE). 
        // Between each attribute - a vec3 representing position - there are (0) bytes, AND the array starts at index 0 (NULL??). 
        //    Ex: data for this attribute looks like: [(NO OFFSET HERE) x0, y0, z0, (NO STRIDE HERE) x1, y1, z1, ...]

    // In general, VAOs look something like this (we just set up the 1st VAO index -- a position array):
    // VAO = [
    //   [ x0, y0, z0, x1, y1, z1, ...], - positions
    //   [ nx0, ny0, nz0, nx1, ny1, nz1, ...], - Normals
    //   ...
    // ]

    glEnableVertexAttribArray(vertTexCoordLocation); // enable the attribute at index 0 within the vao vertex array (position in this case). If not enabled, will not be used when rendering during draw calls
                                                      //glBindBuffer(GL_ARRAY_BUFFER, vbo); // make the vbo active

    glVertexAttribPointer(vertTexCoordLocation, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

    // unbind the VAO
    glBindVertexArray(0);
}


int windowWidth, windowHeight = 0;
HDC hdc = NULL; // device context handle
HGLRC glcxt = NULL; // OpenGL rendering context handle
GLuint fbo = 0;


void resizeWindow(int w, int h) {
    windowWidth = w;
    windowHeight = h;
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

    glGenFramebuffers(1, &fbo);

    return true;
}

// Create VR Structures.
ovrSession session;
ovrEyeRenderDesc eyeRenderDesc[2];
ovrVector3f hmdToEyeViewOffset[2];
ovrLayerEyeFov layer;
ovrHmdDesc hmdDesc;

ovrTexture textures[2];

ovrSwapTextureSet * pTextureSet = 0;

unsigned __int64 frameIndex = 0;

void initOcr() {
    ovrResult result = ovr_Initialize(nullptr);

    // enable SRGB textures in the FrameBuffer. Occulus expects srgb textures when rendering to eyes.
    glEnable(GL_FRAMEBUFFER_SRGB);

    if (OVR_FAILURE(result)) {
        return;
    }

    ovrGraphicsLuid luid;
    result = ovr_Create(&session, &luid); // fails if no rift is plugged in.

    if (OVR_FAILURE(result)) {
        ovr_Shutdown();
        return;
    }

    
    // Set up VR Structures.

    hmdDesc = ovr_GetHmdDesc(session);

    // initialize eye descriptions
    eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
    eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[0]);

    hmdToEyeViewOffset[0] = eyeRenderDesc[0].HmdToEyeViewOffset;
    hmdToEyeViewOffset[1] = eyeRenderDesc[1].HmdToEyeViewOffset;

    // get recommended texture sizes for each eye
    ovrSizei resolution = hmdDesc.Resolution;

    ovrSizei recommenedTex0Size = ovr_GetFovTextureSize(
        session, 
        ovrEye_Left,
        hmdDesc.DefaultEyeFov[0],
        1.0f);

    ovrSizei recommenedTex1Size = ovr_GetFovTextureSize(
        session, 
        ovrEye_Right,
        hmdDesc.DefaultEyeFov[1], 
        1.0f);

    // use recommended sizes for eye frame buffer
    ovrSizei bufferSize;
    bufferSize.w = recommenedTex0Size.w + recommenedTex1Size.w;
    bufferSize.h = max(recommenedTex0Size.h, recommenedTex1Size.h);    

    // create a (swap? because we ping-pong between left-right each frame?) texture set, and retrieve the GL textures from this set
    result = ovr_CreateSwapTextureSetGL(session, GL_SRGB8_ALPHA8, bufferSize.w, bufferSize.h, &pTextureSet);

    if (OVR_FAILURE(result)) {
        ovr_Shutdown();
        return;
    }
    
    layer.Header.Type = ovrLayerType_EyeFov; // seems like a strange line.. since ovrLayerEyeFov could set this by default? since it's typed?
    layer.Header.Flags = 0;
    layer.ColorTexture[0] = pTextureSet; // framebuffer for left eye?
    layer.ColorTexture[1] = pTextureSet; // framebuffer for right eye?
    layer.Fov[0] = eyeRenderDesc[0].Fov;
    layer.Fov[1] = eyeRenderDesc[1].Fov;

    ovrRecti leftEyeViewport = {
        {0, 0},                            // ovrVector2i position;
        {bufferSize.w / 2, bufferSize.h} // ovrSizei size;
    };
    layer.Viewport[0] = leftEyeViewport;

    ovrRecti rightEyeViewport = {
        { bufferSize.w / 2, 0 },            // ovrVector2i position;
        { bufferSize.w / 2, bufferSize.h } // ovrSizei size;
    };

    layer.Viewport[1] = rightEyeViewport;
}

inline void renderTriangle() {

    glUseProgram(shaderProgram); // set our shader program to be the current shaders

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: COMMENT ALL THESE LINES. THEY MAKE TEXTURE MAPPING WORK
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mappedtexture);
    GLuint uniformLocation = glGetUniformLocation(shaderProgram, "tex");
    glUniform1i(uniformLocation, 3);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    glBindVertexArray(vao); // set (vao) to be the current vertex attribute object

    glBindBuffer(GL_ARRAY_BUFFER, vbo); // make this array buffer active (global variable in opengl state maching - ex. array_buffer = vbo
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW); // update values in buffer

    // Draw (3) (GL_TRIANGLES). Get the position data from the currently bound vertex array (vao), and start reading at index (0)
    //   What happens if we have normals as well??
    glDrawArrays(GL_TRIANGLES, 0, 3);

   glBindVertexArray(0);
   glBindTexture(GL_TEXTURE_2D, 0);

}


void renderScene() {

    glClearColor(0.0, 0.0, 0.0, 1.0);

    // Increment to use next texture, just before writing
    pTextureSet->CurrentIndex = (pTextureSet->CurrentIndex+1) % pTextureSet->TextureCount;

    int eye = (pTextureSet->CurrentIndex + 1) % pTextureSet->TextureCount;

    // grab left eye GL texture from the textureset
    ovrGLTexture* tex = (ovrGLTexture*)&pTextureSet->Textures[pTextureSet->CurrentIndex];
    GLuint texId = tex->OGL.TexId;

    double eyeViewportX = layer.Viewport[pTextureSet->CurrentIndex].Pos.x;
    double eyeViewportY = layer.Viewport[pTextureSet->CurrentIndex].Pos.y;
    double eyeViewportWidth = layer.Viewport[pTextureSet->CurrentIndex].Size.w;
    double eyeViewportHeight = layer.Viewport[pTextureSet->CurrentIndex].Size.h;

    // Query the HMD for the current tracking state.
    double displayMidpointSeconds = ovr_GetPredictedDisplayTime(session, frameIndex);
    ovrTrackingState hmdTrackingState = ovr_GetTrackingState(session, displayMidpointSeconds, ovrFalse);

    // calculate the eye poses from the current headpose and hmdToEyeViewOffset
    ovr_CalcEyePoses(hmdTrackingState.HeadPose.ThePose, hmdToEyeViewOffset, layer.RenderPose);

    if (hmdTrackingState.StatusFlags & ovrStatus_PositionConnected) {
        ovrPoseStatef pose = hmdTrackingState.HeadPose;
        // debug code - update one vertex of the triangle to match normalized ovr position
        points[0] =  layer.RenderPose[eye].Position.x;
        points[1] =  layer.RenderPose[eye].Position.y;
        //points[2] =  layer.RenderPose[eye].Position.z;
    }

    // bind a texture to write to
    glBindTexture(GL_TEXTURE_2D, texId);
    // clear the texture (image of 0s)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, eyeViewportWidth, eyeViewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    // filter image
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //glBindTexture(GL_TEXTURE_2D, 0);

    // bind a custom framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // set our texture to our framebuffer's 'colour attachment'? Attach a 2d texture to the fbo
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texId, 0);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear the current color buffer (default framebuffer), depth buffer, and stencil buffer

    glViewport(eyeViewportX, eyeViewportY, eyeViewportWidth, eyeViewportHeight);
    // set up our list of drawBuffers (buffers that GL will draw to?)
    //GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    //glDrawBuffers(1, drawBuffers); // there is 1 draw buffer, contained within drawBuffers

    renderTriangle();

    ovrLayerHeader* layers = &layer.Header;
    ovrResult result = ovr_SubmitFrame(session, 0, nullptr, &layers, 1);

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear the current color buffer (default framebuffer), depth buffer, and stencil buffer
    //glViewport(0, 0, windowWidth, windowHeight);
    //renderTriangle();

    //SwapBuffers(hdc); // windows command -- SUPER BAD FOR RIFT AS (I think) IT'S HANDLED IN ovr_SubmitFrame
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
    initOcr();
    setupScene();

    ShowWindow(hWnd, nCmdShow);
    
    resizeWindow(400, 225);

    UpdateWindow(hWnd);

    MSG msg;
    bool running = true; // keep track of our own 'running' var

    double t = 0;
    double dt = 0;

    double desiredDt = 1 / 75.0;

    LARGE_INTEGER prevCount;
    LARGE_INTEGER count;
    LARGE_INTEGER secondsPerCount;
    
    QueryPerformanceCounter(&prevCount);

    while (running) { 
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { // if there is a message, and handle it (why don't we need to consume it?! -- i.e. GetMessage)
            if (msg.message == WM_QUIT) {
                running = false;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else { // if no message, render!
            QueryPerformanceFrequency(&secondsPerCount);
            QueryPerformanceCounter(&count);

            if (count.QuadPart < prevCount.QuadPart) {
                // overflow - worst case we skip 1 frame.. not ideal
                prevCount = count;
                continue;
            }

            dt = (double)(count.QuadPart - prevCount.QuadPart) / (double)secondsPerCount.QuadPart;
            if (dt < desiredDt) {
                continue;
            }

            renderScene();

            t += dt;
            prevCount = count;
            frameIndex++;
        }
    }

    // destroy occulus session
    ovr_Destroy(session);
    ovr_Shutdown();

    return (int)msg.wParam;
}
