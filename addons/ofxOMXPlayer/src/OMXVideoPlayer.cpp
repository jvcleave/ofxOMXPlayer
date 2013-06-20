/*
 *  OMXVideoPlayer.cpp
 *  openFrameworksLib
 *
 *  Created by jason van cleave on 6/19/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */



#include "OMXVideoPlayer.h"

OMXVideoPlayer::OMXVideoPlayer()
{
	pthread_cond_init(&m_packet_cond, NULL);
	pthread_cond_init(&m_picture_cond, NULL);
	pthread_mutex_init(&m_lock, NULL);
	pthread_mutex_init(&m_lock_decoder, NULL);
}
OMXVideoPlayer::~OMXVideoPlayer()
{
	Close();
	
	pthread_cond_destroy(&m_packet_cond);
	pthread_cond_destroy(&m_picture_cond);
	pthread_mutex_destroy(&m_lock);
	pthread_mutex_destroy(&m_lock_decoder);
}




void OMXVideoPlayer::Lock()
{
    pthread_mutex_lock(&m_lock);
}

void OMXVideoPlayer::UnLock()
{
    pthread_mutex_unlock(&m_lock);
}

void OMXVideoPlayer::LockDecoder()
{
    pthread_mutex_lock(&m_lock_decoder);
}

void OMXVideoPlayer::UnLockDecoder()
{
    pthread_mutex_unlock(&m_lock_decoder);
}

bool OMXVideoPlayer::Decode(OMXPacket *pkt)
{
	if(!pkt)
	{
		return false;
	}
	
	bool ret = false;
	
	if((unsigned long)m_decoder->GetFreeSpace() < pkt->size)
	{
		OMXClock::OMXSleep(10);
	}
	
	if (pkt->dts == DVD_NOPTS_VALUE && pkt->pts == DVD_NOPTS_VALUE)
	{
		pkt->pts = m_pts;
	}else 
	{
		if (pkt->pts == DVD_NOPTS_VALUE)
		{
			pkt->pts = pkt->dts;
		}
	}
	
	if(pkt->pts != DVD_NOPTS_VALUE)
	{
		m_pts = pkt->pts;
		m_pts += m_iVideoDelay;
	}
	
	if((unsigned long)m_decoder->GetFreeSpace() > pkt->size)
	{
		
		m_decoder->Decode(pkt->data, pkt->size, m_pts, m_pts);
		
		m_av_clock->SetVideoClock(m_pts);
		
		Output(m_pts);
		
		ret = true;
	}
	
	return ret;
}

void OMXVideoPlayer::Flush()
{
	Lock();
	LockDecoder();
	m_flush = true;
	while (!m_packets.empty())
	{
		OMXPacket *pkt = m_packets.front(); 
		m_packets.pop_front();
		OMXReader::FreePacket(pkt);
	}
	
	m_iCurrentPts = DVD_NOPTS_VALUE;
	m_cached_size = 0;
	
	if(m_decoder)
	{
		m_decoder->Reset();
	}
	
	m_syncclock = true;
	UnLockDecoder();
	UnLock();
}

void OMXVideoPlayer::Output(double pts)
{
	
	if(m_syncclock)
	{
		double delay = m_FlipTimeStamp - m_av_clock->GetAbsoluteClock();
		if( delay > m_frametime )
		{
			delay = m_frametime;
		}else 
		{
			if( delay < 0 )
			{
				delay = 0;
			}
		}
		
		//printf("OMXEGLImagePlayer - GENERAL_RESYNC(%f, 1) delay %f\n", pts, m_FlipTimeStamp);
		m_av_clock->Discontinuity(pts - delay);
		m_syncclock = false;
	}
	
	double iSleepTime, iClockSleep, iFrameSleep, iPlayingClock, iCurrentClock, iFrameDuration;
	iPlayingClock = m_av_clock->GetClock(iCurrentClock, false); // snapshot current clock
	iClockSleep = pts - iPlayingClock; //sleep calculated by pts to clock comparison
	iFrameSleep = m_FlipTimeStamp - iCurrentClock; // sleep calculated by duration of frame
	iFrameDuration = m_frametime;
	
	// correct sleep times based on speed
	if(m_speed)
	{
		iClockSleep = iClockSleep * DVD_PLAYSPEED_NORMAL / m_speed;
		iFrameSleep = iFrameSleep * DVD_PLAYSPEED_NORMAL / abs(m_speed);
		iFrameDuration = iFrameDuration * DVD_PLAYSPEED_NORMAL / abs(m_speed);
	}
	else
	{
		iClockSleep = 0;
		iFrameSleep = 0;
	}
	
	// dropping to a very low framerate is not correct (it should not happen at all)
	iClockSleep = min(iClockSleep, DVD_MSEC_TO_TIME(500));
	iFrameSleep = min(iFrameSleep, DVD_MSEC_TO_TIME(500));
	
	bool m_stalled = false;
	int m_autosync = 1;
	if( m_stalled )
	{
		iSleepTime = iFrameSleep;
	}
	else
	{
		iSleepTime = iFrameSleep + (iClockSleep - iFrameSleep) / m_autosync;
	}
	
	// present the current pts of this frame to user, and include the actual
	// presentation delay, to allow him to adjust for it
	if( m_stalled )
	{
		m_iCurrentPts = DVD_NOPTS_VALUE;
	}
	else
	{
		m_iCurrentPts = pts - max(0.0, iSleepTime);
	}
	
	m_av_clock->SetPTS(m_iCurrentPts);
	
	// timestamp when we think next picture should be displayed based on current duration
	m_FlipTimeStamp  = iCurrentClock;
	m_FlipTimeStamp += max(0.0, iSleepTime);
	m_FlipTimeStamp += iFrameDuration;
	
	
	
	
	
	/*if(doDebugging)
	 {
	 sprintf(debugInfoBuffer, 
	 "iPlayingClock		 %f	\n\
	 iCurrentClock		 %f	\n\
	 iClockSleep			 %f	\n\
	 iFrameSleep			 %f	\n\
	 iFrameDuration		 %f	\n\
	 WaitAbsolut			 %f	\n\
	 m_FlipTimeStamp		 %f	\n\
	 pts					 %f	\n\
	 currentFrame		 %d	\n\
	 Cached Video		 %8d", 
	 iPlayingClock / DVD_TIME_BASE, 
	 iCurrentClock  / DVD_TIME_BASE,
	 iClockSleep / DVD_TIME_BASE, 
	 iFrameSleep / DVD_TIME_BASE,
	 iFrameDuration / DVD_TIME_BASE, 
	 (iCurrentClock + iSleepTime) / DVD_TIME_BASE, 
	 m_FlipTimeStamp / DVD_TIME_BASE, 
	 pts / DVD_TIME_BASE,
	 (int)((m_FlipTimeStamp / DVD_TIME_BASE)*m_fps),
	 m_cached_size);
	 
	 debugInfo = (string) debugInfoBuffer;
	 }*/
	
	//ofLogVerbose() << debugInfo;
	//EGL WAY - try the while first
	/*if(m_av_clock->GetAbsoluteClock(false) < (iCurrentClock + iSleepTime + DVD_MSEC_TO_TIME(500)) )
	 {
	 //ofLogVerbose() << "OMXEGLImagePlayer::Output returning early";
	 return;
	 }*/
	
	while(m_av_clock->GetAbsoluteClock(false) < (iCurrentClock + iSleepTime + DVD_MSEC_TO_TIME(500)) )
	{
		OMXClock::OMXSleep(10);
	}
  	
	
	m_av_clock->WaitAbsoluteClock((iCurrentClock + iSleepTime));
	
	// guess next frame pts. iDuration is always valid
	if (m_speed != 0)
    {
		m_pts += m_frametime * m_speed / abs(m_speed);
	}
}


bool OMXVideoPlayer::AddPacket(OMXPacket *pkt)
{
	bool ret = false;
	
	if(!pkt)
		return ret;
	
	if(m_bStop || m_bAbort)
		return ret;
	//ofLogVerbose(__func__) << "pkt->size: " << pkt->size;
	if((m_cached_size + pkt->size) < MAX_DATA_SIZE)
	{
		Lock();
		m_cached_size += pkt->size;
		m_packets.push_back(pkt);
		UnLock();
		ret = true;
		pthread_cond_broadcast(&m_packet_cond);
	}
	
	return ret;
}


void OMXVideoPlayer::Process()
{
	OMXPacket *omx_pkt = NULL;
	
	m_pts = 0;
	
	while(!m_bStop && !m_bAbort)
	{
		Lock();
		if(m_packets.empty())
		{
			pthread_cond_wait(&m_packet_cond, &m_lock);
		}
		UnLock();
		
		if(m_bAbort)
		{
			break;
		}
		
		Lock();
		if(m_flush && omx_pkt)
		{
			OMXReader::FreePacket(omx_pkt);
			omx_pkt = NULL;
			m_flush = false;
		}
		else
		{
			if(!omx_pkt && !m_packets.empty())
			{
				omx_pkt = m_packets.front();
				m_cached_size -= omx_pkt->size;
				m_packets.pop_front();
			}
		}
		UnLock();
		
		LockDecoder();
		if(m_flush && omx_pkt)
		{
			OMXReader::FreePacket(omx_pkt);
			omx_pkt = NULL;
			m_flush = false;
		}
		else 
		{
			if(omx_pkt && Decode(omx_pkt))
			{
				OMXReader::FreePacket(omx_pkt);
				omx_pkt = NULL;
			}
		}
		UnLockDecoder();
		
	}
	
	if(omx_pkt)
	{
		OMXReader::FreePacket(omx_pkt);
	}
}

bool OMXVideoPlayer::CloseDecoder()
{
	if(m_decoder)
	{
		delete m_decoder;
	}
	m_decoder   = NULL;
	return true;
}

void OMXVideoPlayer::WaitCompletion()
{
	ofLogVerbose(__func__) << "OMXVideoPlayer WaitCompletion";
	
	if(!m_decoder)
	{
		return;
	}
	
	while(true)
	{
		Lock();
		if(m_packets.empty())
		{
			//ofLogVerbose() << "packets empty"; //maybe crashing with string err
			UnLock();
			break;
		}
		UnLock();
		OMXClock::OMXSleep(50);
	}
	
	m_decoder->WaitCompletion();
}
bool OMXVideoPlayer::Close()
{
	m_bAbort  = true;
	m_flush   = true;
	
	Flush();
	
	if(ThreadHandle())
	{
		Lock();
		pthread_cond_broadcast(&m_packet_cond);
		UnLock();
		
		StopThread();
	}
	
	CloseDecoder();
	
	m_dllAvUtil.Unload();
	m_dllAvCodec.Unload();
	m_dllAvFormat.Unload();
	
	m_open          = false;
	m_stream_id     = -1;
	m_iCurrentPts   = DVD_NOPTS_VALUE;
	m_pStream       = NULL;
	m_pts           = 0;
	m_syncclock     = false;
	m_speed         = DVD_PLAYSPEED_NORMAL;
	
	return true;
}
