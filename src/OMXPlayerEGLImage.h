#pragma once
#include "OMXPlayerVideoBase.h"


#include "OMXEGLImage.h"


#include <deque>
#include <sys/types.h>



class OMXPlayerEGLImage : public OMXPlayerVideoBase
{
	public:
		OMXPlayerEGLImage();
		~OMXPlayerEGLImage();

		bool open(OMXStreamInfo& hints, OMXClock *av_clock, EGLImageKHR eglImage);
		bool openDecoder();
		bool close();

		ofxOMXPlayerSettings settings;
		OMXEGLImage*				eglImageDecoder;
		EGLImageKHR eglImage;
};

