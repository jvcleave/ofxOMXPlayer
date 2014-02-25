#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"
#include "OMXDecoderBase.h"

class OMXEGLImage : public OMXDecoderBase
{
public:
	OMXEGLImage();
	
	
	bool Open(COMXStreamInfo &hints, OMXClock *clock, EGLImageKHR eglImage);
	bool PortSettingsChanged() {return true;};
	int  Decode(uint8_t *pData, int iSize, double dts, double pts);
	int  Decode(uint8_t *pData, int iSize, double pts);
	

	
};
