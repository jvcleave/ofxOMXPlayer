/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */



#include "OMXPlayerVideo.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>


#include "XMemUtils.h"


OMXPlayerVideo::OMXPlayerVideo()
{
	m_hdmi_clock_sync = false;
	nonTextureDecoder = NULL;


}
OMXPlayerVideo::~OMXPlayerVideo()
{
	//ofLogVerbose(__func__) << "START";

	Close();

	pthread_cond_destroy(&m_packet_cond);
	pthread_mutex_destroy(&m_lock);
	pthread_mutex_destroy(&m_lock_decoder);
	//ofLogVerbose(__func__) << "END";
}

bool OMXPlayerVideo::Open(OMXStreamInfo& hints, OMXClock *av_clock, bool deinterlace, bool hdmi_clock_sync, float display_aspect)
{
	//ofLogVerbose(__func__) << "OMXPlayerVideo Open";

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
	m_Deinterlace = deinterlace;
	m_display_aspect = display_aspect;
	m_iCurrentPts = DVD_NOPTS_VALUE;
	doAbort      = false;
	doFlush       = false;
	cachedSize = 0;
	m_iVideoDelay = 0;
	m_hdmi_clock_sync = hdmi_clock_sync;
	speed       = DVD_PLAYSPEED_NORMAL;
	

	m_FlipTimeStamp = omxClock->getAbsoluteClock();

	if(!OpenDecoder())
	{
		Close();
		return false;
	}

	Create();

	m_open        = true;

	return true;
}


bool OMXPlayerVideo::OpenDecoder()
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
		ofLog(OF_LOG_VERBOSE, "Invalid framerate %d, using forced 25fps and just trust timestamps\n", (int)m_fps);
		m_fps = 25;
	}

	m_frametime = (double)DVD_TIME_BASE / m_fps;

	nonTextureDecoder = new OMXVideo();
	
	nonTextureDecoder->setDisplayRect(displayRect);

	decoder = (OMXDecoderBase*)nonTextureDecoder;
	if(!nonTextureDecoder->Open(omxStreamInfo, omxClock, m_display_aspect, m_Deinterlace, m_hdmi_clock_sync))
	{

		CloseDecoder();
		return false;
	}
	
	stringstream info;
	info << "Video width: "	<<	omxStreamInfo.width					<< "\n";
	info << "Video height: "	<<	omxStreamInfo.height					<< "\n";
	info << "Video profile: "	<<	omxStreamInfo.profile					<< "\n";
	info << "Video fps: "		<<	m_fps							<< "\n";
	//ofLogVerbose(__func__) << "\n" << info;

	/*ofLog(OF_LOG_VERBOSE, "Video codec %s width %d height %d profile %d fps %f\n",
		decoder->GetDecoderName().c_str() , omxStreamInfo.width, omxStreamInfo.height, omxStreamInfo.profile, m_fps);*/




	return true;
}

bool OMXPlayerVideo::Close()
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

		StopThread("OMXPlayerVideo");
	}
	
	if (nonTextureDecoder && !isExiting)
	{
		//ofLogVerbose(__func__) << "PRE DELETE nonTextureDecoder";
		delete nonTextureDecoder;
		nonTextureDecoder = NULL;
		//ofLogVerbose(__func__) << "POST DELETE nonTextureDecoder";
	}

	m_open          = false;
	streamID     = -1;
	m_iCurrentPts   = DVD_NOPTS_VALUE;
	speed         = DVD_PLAYSPEED_NORMAL;

	//ofLogVerbose(__func__) << " END";
	return true;
}

bool OMXPlayerVideo::validateDisplayRect(ofRectangle& rectangle)
{
	//ofLogVerbose(__func__) << "displayRect: " << displayRect;
	//ofLogVerbose(__func__) << "rectangle: " << rectangle;
	if (displayRect == rectangle) 
	{
		return false;
	}
	displayRect = rectangle;
	return true;
}
void OMXPlayerVideo::setDisplayRect(ofRectangle& rectangle)
{
	if (ThreadHandle()) 
	{
		Lock();
			if(validateDisplayRect(rectangle))
			{
				LockDecoder();
					nonTextureDecoder->setDisplayRect(displayRect);
				UnLockDecoder();
			}
		UnLock();
	}else 
	{
		validateDisplayRect(rectangle);
	}
}
