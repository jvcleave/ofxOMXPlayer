#pragma once
#include "OMXPlayerVideoBase.h"


#include "OMXEGLImage.h"


#include <deque>
#include <sys/types.h>



class OMXPlayerEGLImage : public OMXPlayerVideoBase
{
public:
	OMXPlayerEGLImage();
	bool Open(COMXStreamInfo &hints, OMXClock *av_clock);
	bool OpenDecoder();
	
	OMXEGLImage*				eglImageDecoder;
};

