#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"
#include "OMXDecoderBase.h"
#include "GlobalEGLContainer.h"

class OMXEGLImage : public OMXDecoderBase
{
public:
	OMXEGLImage();
	bool Open(COMXStreamInfo &hints, OMXClock *clock);
	int  Decode(uint8_t *pData, int iSize, double dts, double pts);
	
	OMX_BUFFERHEADERTYPE* eglBuffer;
	
	
	
	
};
