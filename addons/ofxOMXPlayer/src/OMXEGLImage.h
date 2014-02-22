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
	bool PortSettingsChanged() {return true;};
	int  Decode(uint8_t *pData, int iSize, double dts, double pts);
	int  Decode(uint8_t *pData, int iSize, double pts);
	OMX_BUFFERHEADERTYPE* eglBuffer;
	
	
	
	
};
