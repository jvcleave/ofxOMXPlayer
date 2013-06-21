#pragma once
#include "OMXVideoPlayer.h"


#include "OMXEGLImage.h"


#include <deque>
#include <sys/types.h>



class OMXEGLImagePlayer : public OMXVideoPlayer
{
public:
	OMXEGLImagePlayer();
	bool Open(COMXStreamInfo &hints, OMXClock *av_clock, EGLImageKHR eglImage_);
	bool OpenDecoder();
	EGLImageKHR eglImage;
	
	OMXEGLImage*				eglImageDecoder;
};

