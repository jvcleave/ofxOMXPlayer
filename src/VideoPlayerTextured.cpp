#include "VideoPlayerTextured.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>


#include "XMemUtils.h"


VideoPlayerTextured::VideoPlayerTextured()
{
	textureDecoder = NULL;
}


VideoPlayerTextured::~VideoPlayerTextured()
{
    close();
}

bool VideoPlayerTextured::open(StreamInfo& hints, OMXClock *av_clock, EGLImageKHR eglImage)
{

	this->eglImage = eglImage;

	if (!av_clock)
	{
		return false;
	}

	if(ThreadHandle())
	{
		close();
	}


	omxStreamInfo       = hints;
	omxClock    = av_clock;
	fps         = 25.0f;
	frameTime   = 0;
	currentPTS = DVD_NOPTS_VALUE;
	doAbort      = false;
	doFlush       = false;
	cachedSize = 0;
	speed       = DVD_PLAYSPEED_NORMAL;
	timeStampAdjustment = omxClock->getAbsoluteClock();

	if(!openDecoder())
	{
		close();
		return false;
	}

	Create();


	isOpen        = true;

	return true;
}


bool VideoPlayerTextured::openDecoder()
{
	if (omxStreamInfo.fpsrate && omxStreamInfo.fpsscale)
	{
		fps = DVD_TIME_BASE / OMXReader::normalizeFrameduration((double)DVD_TIME_BASE * omxStreamInfo.fpsscale / omxStreamInfo.fpsrate);
	}
	else
	{
		fps = 25;
	}

	if( fps > 100 || fps < 5 )
	{
		//ofLogVerbose(__func__) << "Invalid framerate " << fps  << " using forced 25fps and just trust timestamps";
		fps = 25;
	}

	frameTime = (double)DVD_TIME_BASE / fps;

	if (!textureDecoder)
	{
		textureDecoder = new VideoDecoderTextured();

	}

	decoder = (BaseVideoDecoder*)textureDecoder;

	if(!textureDecoder->open(omxStreamInfo, omxClock, eglImage))
	{
        delete textureDecoder;
        textureDecoder = NULL;
        decoder = NULL;
		return false;
	}

	stringstream info;
	info << "Video width: "     <<	omxStreamInfo.width     << "\n";
	info << "Video height: "    <<	omxStreamInfo.height    << "\n";
	info << "Video profile: "   <<	omxStreamInfo.profile   << "\n";
	info << "Video fps: "		<<	fps                     << "\n";
	ofLogVerbose(__func__) << "\n" << info.str();


	return true;
}

void VideoPlayerTextured::close()
{
	doAbort  = true;
	doFlush   = true;
	
	flush();
	
	if(ThreadHandle())
	{
		lock();
            pthread_cond_broadcast(&m_packet_cond);
		unlock();

		StopThread("VideoPlayerTextured");
	}

	if (textureDecoder)
	{
		delete textureDecoder;
		textureDecoder = NULL;
	};

	isOpen      = false;
	currentPTS  = DVD_NOPTS_VALUE;
	speed       = DVD_PLAYSPEED_NORMAL;

}
