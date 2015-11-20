#include "OMXPlayerEGLImage.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>


#include "XMemUtils.h"


OMXPlayerEGLImage::OMXPlayerEGLImage()
{
	eglImageDecoder = NULL;
}

OMXPlayerEGLImage::~OMXPlayerEGLImage()
{
	Close();
}


bool OMXPlayerEGLImage::Open(OMXStreamInfo& hints, OMXClock *av_clock, EGLImageKHR eglImage)
{

	//ofLogVerbose(__func__) << " OMXPlayerEGLImage Open";
	this->eglImage = eglImage;

	if (!av_clock)
	{
		return false;
	}

	if(ThreadHandle())
	{
		Close();
	}


	omxStreamInfo       = hints;
	omxClock    = av_clock;
	m_fps         = 25.0f;
	m_frametime   = 0;
	currentPTS = DVD_NOPTS_VALUE;
	doAbort      = false;
	doFlush       = false;
	cachedSize = 0;
	m_iVideoDelay = 0;
	speed       = DVD_PLAYSPEED_NORMAL;
	m_FlipTimeStamp = omxClock->getAbsoluteClock();

	if(!openDecoder())
	{
		Close();
		return false;
	}

	Create();


	isOpen        = true;

	return true;
}


bool OMXPlayerEGLImage::openDecoder()
{
	if (omxStreamInfo.fpsrate && omxStreamInfo.fpsscale)
	{
		m_fps = DVD_TIME_BASE / OMXReader::NormalizeFrameduration((double)DVD_TIME_BASE * omxStreamInfo.fpsscale / omxStreamInfo.fpsrate);
	}
	else
	{
		m_fps = 25;
	}

	if( m_fps > 100 || m_fps < 5 )
	{
		//ofLogVerbose(__func__) << "Invalid framerate " << m_fps  << " using forced 25fps and just trust timestamps";
		m_fps = 25;
	}

	m_frametime = (double)DVD_TIME_BASE / m_fps;

	if (!eglImageDecoder)
	{
		eglImageDecoder = new OMXEGLImage();

	}

	decoder = (OMXDecoderBase*)eglImageDecoder;

	if(!eglImageDecoder->Open(omxStreamInfo, omxClock, eglImage))
	{
		closeDecoder();
		return false;
	}

	stringstream info;
	info << "Video width: "	<<	omxStreamInfo.width					<< "\n";
	info << "Video height: "	<<	omxStreamInfo.height					<< "\n";
	info << "Video profile: "	<<	omxStreamInfo.profile					<< "\n";
	info << "Video fps: "		<<	m_fps							<< "\n";
	//ofLogVerbose(__func__) << "\n" << info.str();


	/*ofLog(OF_LOG_VERBOSE, "Video codec %s width %d height %d profile %d fps %f\n",
		  decoder->GetDecoderName().c_str() , omxStreamInfo.width, omxStreamInfo.height, omxStreamInfo.profile, m_fps);*/


	return true;
}

bool OMXPlayerEGLImage::Close()
{
	//ofLogVerbose(__func__) << " START, isExiting:" << isExiting;
	doAbort  = true;
	doFlush   = true;
	
	Flush();
	

	if(ThreadHandle())
	{
		Lock();
		//ofLogVerbose(__func__) << "WE ARE STILL THREADED";
		pthread_cond_broadcast(&m_packet_cond);
		UnLock();

		StopThread("OMXPlayerEGLImage");
	}

	if (eglImageDecoder)
	{
		//ofLogVerbose(__func__) << "PRE DELETE eglImageDecoder";
		delete eglImageDecoder;
		eglImageDecoder = NULL;
		//ofLogVerbose(__func__) << "POST DELETE eglImageDecoder";
	};


	isOpen          = false;
	streamID     = -1;
	currentPTS   = DVD_NOPTS_VALUE;
	speed         = DVD_PLAYSPEED_NORMAL;

	//ofLogVerbose(__func__) << " END";
	return true;
}
