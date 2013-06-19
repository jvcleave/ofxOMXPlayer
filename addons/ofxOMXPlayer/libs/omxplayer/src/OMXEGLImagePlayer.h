#pragma once
#include "OMXVideoPlayer.h"


#include "OMXEGLImage.h"


#include <deque>
#include <sys/types.h>



class OMXEGLImagePlayer : public OMXVideoPlayer
{


	
	

	//char debugInfoBuffer [1024];
	


public:
	OMXEGLImagePlayer();
	~OMXEGLImagePlayer();
	bool Open(COMXStreamInfo &hints, OMXClock *av_clock, EGLImageKHR eglImage_);
	bool OpenDecoder();
	//void SetSpeed(int iSpeed);
	//int GetSpeed(){return m_speed;};
	EGLImageKHR eglImage;
	OMXEGLImage                 *m_decoder;
	//string debugInfo;
	//bool doDebugging;
	void Output(double pts);
};

