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


bool OMXPlayerEGLImage::Open(COMXStreamInfo& hints, OMXClock *av_clock, EGLImageKHR eglImage)
{

	ofLogVerbose(__func__) << " OMXPlayerEGLImage Open";
	this->eglImage = eglImage;

	if (!av_clock)
	{
		return false;
	}

	if(ThreadHandle())
	{
		Close();
	}


	m_hints       = hints;
	m_av_clock    = av_clock;
	m_fps         = 25.0f;
	m_frametime   = 0;
	m_iCurrentPts = DVD_NOPTS_VALUE;
	m_bAbort      = false;
	m_flush       = false;
	m_cached_size = 0;
	m_iVideoDelay = 0;
	m_speed       = DVD_PLAYSPEED_NORMAL;
	m_FlipTimeStamp = m_av_clock->GetAbsoluteClock();

	if(!OpenDecoder())
	{
		Close();
		return false;
	}

	Create();


	m_open        = true;

	return true;
}


bool OMXPlayerEGLImage::OpenDecoder()
{
	if (m_hints.fpsrate && m_hints.fpsscale)
	{
		m_fps = DVD_TIME_BASE / OMXReader::NormalizeFrameduration((double)DVD_TIME_BASE * m_hints.fpsscale / m_hints.fpsrate);
	}
	else
	{
		m_fps = 25;
	}

	if( m_fps > 100 || m_fps < 5 )
	{
		ofLogVerbose(__func__) << "Invalid framerate " << m_fps  << " using forced 25fps and just trust timestamps";
		m_fps = 25;
	}

	m_frametime = (double)DVD_TIME_BASE / m_fps;

	if (!eglImageDecoder)
	{
		eglImageDecoder = new OMXEGLImage();

	}

	m_decoder = (OMXDecoderBase*)eglImageDecoder;

	if(!eglImageDecoder->Open(m_hints, m_av_clock, eglImage))
	{
		CloseDecoder();
		return false;
	}

	stringstream info;
	info << "Video codec: "	<<	m_decoder->GetDecoderName()		<< "\n";
	info << "Video width: "	<<	m_hints.width					<< "\n";
	info << "Video height: "	<<	m_hints.height					<< "\n";
	info << "Video profile: "	<<	m_hints.profile					<< "\n";
	info << "Video fps: "		<<	m_fps							<< "\n";
	ofLogVerbose(__func__) << "\n" << info.str();


	/*ofLog(OF_LOG_VERBOSE, "Video codec %s width %d height %d profile %d fps %f\n",
		  m_decoder->GetDecoderName().c_str() , m_hints.width, m_hints.height, m_hints.profile, m_fps);*/


	return true;
}

bool OMXPlayerEGLImage::Close()
{
	ofLogVerbose(__func__) << " START, isExiting:" << isExiting;
	m_bAbort  = true;
	m_flush   = true;
	
	Flush();
	

	if(ThreadHandle())
	{
		Lock();
		ofLogVerbose(__func__) << "WE ARE STILL THREADED";
		pthread_cond_broadcast(&m_packet_cond);
		UnLock();

		StopThread("OMXPlayerEGLImage");
	}

	if (eglImageDecoder)
	{
		ofLogVerbose(__func__) << "PRE DELETE eglImageDecoder";
		delete eglImageDecoder;
		eglImageDecoder = NULL;
		ofLogVerbose(__func__) << "POST DELETE eglImageDecoder";
	};


	m_open          = false;
	m_stream_id     = -1;
	m_iCurrentPts   = DVD_NOPTS_VALUE;
	m_pStream       = NULL;
	m_speed         = DVD_PLAYSPEED_NORMAL;

	ofLogVerbose(__func__) << " END";
	return true;
}
