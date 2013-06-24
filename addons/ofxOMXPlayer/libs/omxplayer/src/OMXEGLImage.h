#pragma once

#include "ofMain.h"

#include "OMXDecoderBase.h"

class OMXEGLImage : public OMXDecoderBase
{
public:
	OMXEGLImage();
	
	bool Open(COMXStreamInfo &hints, OMXClock *clock, EGLImageKHR eglImage_);
	void Close();
	int  Decode(uint8_t *pData, int iSize, double dts, double pts);
	

	EGLImageKHR eglImage;
	OMX_BUFFERHEADERTYPE* eglBuffer;
	char debugInfoBuffer [128];
	string debugInfo;
	bool doDebugging;





	

	
};
