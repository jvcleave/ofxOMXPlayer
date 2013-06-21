#pragma once
#include "OMXPlayerVideoBase.h"


#include "OMXEGLImage.h"


#include <deque>
#include <sys/types.h>



class OMXPlayerEGLImage : public OMXPlayerVideoBase
{
public:
	OMXPlayerEGLImage();
	bool Open(COMXStreamInfo &hints, OMXClock *av_clock, EGLImageKHR eglImage_);
	bool OpenDecoder();
	EGLImageKHR eglImage;
	
	OMXEGLImage*				eglImageDecoder;
};

