#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"
#include "OMXDecoderBase.h"
#include "GlobalEGLContainer.h"

class OMXEGLImage : public OMXDecoderBase
{
public:
	OMXEGLImage();
	~OMXEGLImage();
	bool Open(COMXStreamInfo &hints, OMXClock *clock);
	void Close();
	int  Decode(uint8_t *pData, int iSize, double dts, double pts);
	
	/*ofAppEGLWindow*		appEGLWindow;
	EGLDisplay			display;
	EGLContext			context;
	
	EGLImageKHR			eglImage;
	ofTexture			tex;
	GLuint				textureID;*/
	OMX_BUFFERHEADERTYPE* eglBuffer;
	
	
	
	
};
