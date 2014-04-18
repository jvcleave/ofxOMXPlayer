#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"
#include "OMXDecoderBase.h"

class OMXEGLImage : public OMXDecoderBase
{
	public:
		OMXEGLImage();
		~OMXEGLImage();

		bool Open(COMXStreamInfo& hints, OMXClock *clock, EGLImageKHR eglImage);
		
		bool  Decode(uint8_t *pData, int iSize, double pts);

	
};
