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

		bool Open(OMXStreamInfo& hints, OMXClock *av_clock, EGLImageKHR eglImage);
		bool openDecoder();
		bool Close();

		ofxOMXPlayerSettings settings;
		OMXEGLImage*				eglImageDecoder;
		EGLImageKHR eglImage;
};

