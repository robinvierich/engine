#pragma comment(lib, "glew32.lib")

#include <iostream>
#include <sstream>
#include <vector>

#include <GL\glew.h>
#include <GL\wglew.h>

#include "frame.h"
#include "utils.h"

#include "ovr.h"

#include "Matrix.h"
#include "Vector.h"

const char* vertex_shader =
"#version 400\n"
"in vec3 vp;"
"in vec2 vertTexCoord;"
"out vec2 fragTexCoord;"
"uniform mat4 projectionMatrix;"
"uniform mat4 cameraMatrix;"
"uniform mat4 modelMatrix;"
"void main () {"
"  fragTexCoord = vertTexCoord;"
"  gl_Position = projectionMatrix * cameraMatrix * modelMatrix * vec4(vp, 1.0);"
"}";

const char* fragment_shader =
"#version 400\n"
"uniform sampler2D tex;" //this is the texture
"in vec2 fragTexCoord;" //this is the texture coord
"out vec4 frag_colour;"
"void main () {"
"   frag_colour = texture(tex, fragTexCoord);"
"}";

GLuint vbo = 0; // Vertex buffer object. Stores some vertex data. Can be position, normal, UV, ... In this case, position.
GLuint vao = 0; // Vertex attribute object. Stores a bunch of VBOs together in the same object (for efficiency)

GLuint vShader = 0;
GLuint fShader = 0;
GLuint shaderProgram = 0;

float points[] = { // some points to draw
//    x     y     z       u     v
	0.0f, 0.0f, -2.1f,   0.0f, 0.0f,
	0.5f, 0.5f, -2.1f,   1.0f, 1.0f,
	0.5f, 0.0f, -2.1f,   1.0f, 0.0f,
};

GLuint mappedtexture;

const float PI = 3.14159;

// TODO: DOCUMENT THIS FUNCTION!!
Matrix44f createProjectionMatrix(float fov, float farPlane, float nearPlane) {
    const float zScale = -farPlane / (farPlane - nearPlane);
    const float zTransform = -(farPlane * nearPlane) / (farPlane - nearPlane);

    const float fovScale = 1.0f / tan((fov / 2.0f) * (PI / 180.0f));

    return Matrix44f(
        fovScale,     0.0f,   0.0f,       0.0f,
        0.0f,     fovScale,   0.0f,       0.0f,
        0.0f,         0.0f, zScale, zTransform,
        0.0f,         0.0f,  -1.0f,       0.0f
    );
}

Matrix44f modelToWorld = Matrix44f::Identity();
Matrix44f cameraToWorld = Matrix44f::Identity();
Matrix44f cameraToScreenProj = createProjectionMatrix(60, 1, 100);

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
    // Create uniforms for model/camera/projection matrices
    GLuint modelMatrixUniformLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
    GLuint cameraMatrixUniformLocation = glGetUniformLocation(shaderProgram, "cameraMatrix");
    GLuint projectionMatrixUniformLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
    
    glUniformMatrix4fv(modelMatrixUniformLocation, 1, GL_FALSE, modelToWorld.data());
    glUniformMatrix4fv(cameraMatrixUniformLocation, 1, GL_FALSE, cameraToWorld.data());
    glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, cameraToScreenProj.data());


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// LOAD A TEXTURE
	int width = 256, height = 256;//0;
	std::vector<unsigned char> pixels;
	loadBmp("C:\\code\\engine\\x64\\Debug\\smiley.bmp", &pixels, &width, &height);

	glGenTextures(1, &mappedtexture);

	glBindTexture(GL_TEXTURE_2D, mappedtexture);
	// clear the texture (image of zeros)

	unsigned char* pixelData = pixels.data();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB4, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);

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

HDC hdc = NULL; // device context handle
HGLRC glcxt = NULL; // OpenGL rendering context handle

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
	}
	else {
		glcxt = tempOpenGLContext; // if this fails, use default OpenGL 2.1 context
	}

	int glVersion[2] = { -1, -1 }; // default version values
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]); // use GLEW to get the Major version of OpenGL we're using
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]); // use GLEW to get the Minor version of OpenGL we're using

	DBOUT("Using OpenGL " << glVersion[0] << "." << glVersion[1] << std::endl);

	return true;
}

inline void renderTriangle(Frame& frame) {

	glUseProgram(shaderProgram); // set our shader program to be the current shaders

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TODO: COMMENT ALL THESE LINES. THEY MAKE TEXTURE MAPPING WORK
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, mappedtexture);
	GLuint uniformLocation = glGetUniformLocation(shaderProgram, "tex");
	glUniform1i(uniformLocation, 3);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	glBindVertexArray(vao); // set (vao) to be the current vertex attribute object


    // send these guys down to the vshader:
    //   model->world
    //   -1 camera->world (i.e. world->camera)
    //   camera->screen


	glBindBuffer(GL_ARRAY_BUFFER, vbo); // make this array buffer active (global variable in opengl state maching - ex. array_buffer = vbo
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW); // update values in buffer

    // Draw (3) (GL_TRIANGLES). Get the position data from the currently bound vertex array (vao), and start reading at index (0)
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

}


void initRenderer(HWND hWnd) {
	initOpenGL(hWnd);
	initOvr();
	setupScene();
}

void shutdownRenderer() {
	shutdownOvr();
}


void updateRenderer(Frame& frame, int windowWidth, int windowHeight) {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear the current color buffer (default framebuffer), depth buffer, and stencil buffer

	//setupOvrFrame(frame);

	// set up our list of drawBuffers (buffers that GL will draw to?)
	//GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	//glDrawBuffers(1, drawBuffers); // there is 1 draw buffer, contained within drawBuffers

	// renderTriangle();

	//submitOvrFrame();

	// glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear the current color buffer (default framebuffer), depth buffer, and stencil buffer
	glViewport(0, 0, windowWidth, windowHeight);
	renderTriangle(frame);

	SwapBuffers(hdc); // windows command -- SUPER BAD FOR RIFT AS IT'S HANDLED IN ovr_SubmitFrame
}
