#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#include <GL\glew.h>
#include <GL\wglew.h>

#include "frame.h"

// Create VR Structures.
ovrSession session;
ovrEyeRenderDesc eyeRenderDesc[2];
ovrVector3f hmdToEyeViewOffset[2];
ovrLayerEyeFov layer;
ovrHmdDesc hmdDesc;

ovrTexture textures[2];

ovrSwapTextureSet * pTextureSet = 0;

GLuint fbo = 0;

void initOvr() {
	ovrResult result = ovr_Initialize(nullptr);

	// enable SRGB textures in the FrameBuffer. Occulus expects srgb textures when rendering to eyes.
	glEnable(GL_FRAMEBUFFER_SRGB);

	// generate a framebuffer object
	glGenFramebuffers(1, &fbo);

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
		{ 0, 0 },                            // ovrVector2i position;
		{ bufferSize.w / 2, bufferSize.h } // ovrSizei size;
	};
	layer.Viewport[0] = leftEyeViewport;

	ovrRecti rightEyeViewport = {
		{ bufferSize.w / 2, 0 },            // ovrVector2i position;
		{ bufferSize.w / 2, bufferSize.h } // ovrSizei size;
	};

	layer.Viewport[1] = rightEyeViewport;
}


void setupOvrFrame(Frame& frame) {
	// Increment to use next texture, just before writing
	pTextureSet->CurrentIndex = (pTextureSet->CurrentIndex + 1) % pTextureSet->TextureCount;

	int eye = (pTextureSet->CurrentIndex + 1) % pTextureSet->TextureCount;

	// grab left eye GL texture from the textureset
	ovrGLTexture* tex = (ovrGLTexture*)&pTextureSet->Textures[pTextureSet->CurrentIndex];
	GLuint texId = tex->OGL.TexId;

	double eyeViewportX = layer.Viewport[pTextureSet->CurrentIndex].Pos.x;
	double eyeViewportY = layer.Viewport[pTextureSet->CurrentIndex].Pos.y;
	double eyeViewportWidth = layer.Viewport[pTextureSet->CurrentIndex].Size.w;
	double eyeViewportHeight = layer.Viewport[pTextureSet->CurrentIndex].Size.h;

	// Query the HMD for the current tracking state.
	double displayMidpointSeconds = ovr_GetPredictedDisplayTime(session, frame.frameIndex);
	ovrTrackingState hmdTrackingState = ovr_GetTrackingState(session, displayMidpointSeconds, ovrFalse);

	// calculate the eye poses from the current headpose and hmdToEyeViewOffset
	ovr_CalcEyePoses(hmdTrackingState.HeadPose.ThePose, hmdToEyeViewOffset, layer.RenderPose);

	//if (hmdTrackingState.StatusFlags & ovrStatus_PositionConnected) {
	//	ovrPoseStatef pose = hmdTrackingState.HeadPose;
	//	// debug code - update one vertex of the triangle to match normalized ovr position
	//	points[0] = layer.RenderPose[eye].Position.x;
	//	points[1] = layer.RenderPose[eye].Position.y;
	//	//points[2] =  layer.RenderPose[eye].Position.z;
	//}

	// bind a texture to write to
	glBindTexture(GL_TEXTURE_2D, texId);
	// clear the texture (image of 0s)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, eyeViewportWidth, eyeViewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// filter image
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// bind a custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// set our texture to our framebuffer's 'colour attachment'? Attach a 2d texture to the fbo
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texId, 0);

	glViewport(eyeViewportX, eyeViewportY, eyeViewportWidth, eyeViewportHeight);
}

void submitOvrFrame() {
	ovrLayerHeader* layers = &layer.Header;
	ovrResult result = ovr_SubmitFrame(session, 0, nullptr, &layers, 1);
}

void shutdownOvr() {
	// destroy occulus session
	ovr_Destroy(session);
	ovr_Shutdown();
}
