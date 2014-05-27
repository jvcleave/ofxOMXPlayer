#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"
#include "OMXDecoderBase.h"

class OMXEGLImage : public OMXDecoderBase
{
	public:
		OMXEGLImage();

		bool Open(COMXStreamInfo& hints, OMXClock *clock, EGLImageKHR eglImage);
		
		bool  Decode(uint8_t *pData, int iSize, double pts);

		int getCurrentFrame();
		void resetFrameCounter();
	static OMX_ERRORTYPE onFillBufferDone(OMX_HANDLETYPE hComponent,
										  OMX_PTR pAppData,
										  OMX_BUFFERHEADERTYPE* pBuffer);
private:
	int frameCounter;
	int frameOffset;
};
