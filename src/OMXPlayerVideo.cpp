#include "OMXPlayerVideo.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>


#include "linux/XMemUtils.h"

//#define MAX_DATA_SIZE    40 * 1024 * 1024

OMXPlayerVideo::OMXPlayerVideo()
{
	m_open          = false;
	m_stream_id     = -1;
	m_pStream       = NULL;
	m_av_clock      = NULL;
	m_decoder       = NULL;
	m_fps           = 25.0f;
	m_flush         = false;
	m_cached_size   = 0;
	m_iVideoDelay   = 0;
	m_pts           = 0;
	m_syncclock     = true;
	m_speed         = DVD_PLAYSPEED_NORMAL;
	int multiplier	= 40; //omxplayer default is 10
	maxDataSize		= multiplier * 1024 * 1024;
	doDebugging = false;
	lastFrameDuration = 0;
	pthread_cond_init(&m_packet_cond, NULL);
	pthread_cond_init(&m_picture_cond, NULL);
	pthread_mutex_init(&m_lock, NULL);
	pthread_mutex_init(&m_lock_decoder, NULL);
}

OMXPlayerVideo::~OMXPlayerVideo()
{
  Close();

  pthread_cond_destroy(&m_packet_cond);
  pthread_cond_destroy(&m_picture_cond);
  pthread_mutex_destroy(&m_lock);
  pthread_mutex_destroy(&m_lock_decoder);
}

void OMXPlayerVideo::Lock()
{
    pthread_mutex_lock(&m_lock);
}

void OMXPlayerVideo::UnLock()
{
    pthread_mutex_unlock(&m_lock);
}

void OMXPlayerVideo::LockDecoder()
{
    pthread_mutex_lock(&m_lock_decoder);
}

void OMXPlayerVideo::UnLockDecoder()
{
    pthread_mutex_unlock(&m_lock_decoder);
}


bool OMXPlayerVideo::Open(COMXStreamInfo &hints, OMXClock *av_clock, EGLImageKHR eglImage_)
{
	eglImage = eglImage_;
	ofLogVerbose() << "OMXPlayerVideo::maxDataSize may need to be reduced for 256 boards, memory intensive apps";

  if (!m_dllAvUtil.Load() || !m_dllAvCodec.Load() || !m_dllAvFormat.Load() || !av_clock)
  {
	  return false;
  }

	if(ThreadHandle())
	{
		Close();
	}

	m_dllAvFormat.av_register_all();

	m_hints       = hints;
	m_av_clock    = av_clock;
	m_fps         = 25.0f;
	m_frametime   = 0;
	m_iCurrentPts = DVD_NOPTS_VALUE;
	m_bAbort      = false;
	m_flush       = false;
	m_cached_size = 0;
	m_iVideoDelay = 0;
	m_pts         = 0;
	m_syncclock   = true;
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

bool OMXPlayerVideo::Close()
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

void OMXPlayerVideo::Output(double pts)
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

		//printf("OMXPlayerVideo - GENERAL_RESYNC(%f, 1) delay %f\n", pts, m_FlipTimeStamp);
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
	
	if(doDebugging)
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
	}

	//ofLogVerbose() << debugInfo;
	
	if(m_av_clock->GetAbsoluteClock(false) < (iCurrentClock + iSleepTime + DVD_MSEC_TO_TIME(500)) )
	{
		//ofLogVerbose() << "OMXPlayerVideo::Output returning early";
		return;
	}
	
  	

	m_av_clock->WaitAbsoluteClock((iCurrentClock + iSleepTime));

	// guess next frame pts. iDuration is always valid
	if (m_speed != 0)
    {
		m_pts += m_frametime * m_speed / abs(m_speed);
	}
}

bool OMXPlayerVideo::Decode(OMXPacket *pkt)
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

void OMXPlayerVideo::Process()
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


void OMXPlayerVideo::Flush()
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
    m_decoder->Reset();
  m_syncclock = true;
  UnLockDecoder();
  UnLock();
}

bool OMXPlayerVideo::AddPacket(OMXPacket *pkt)
{
  bool ret = false;

  if(!pkt)
    return ret;

  if(m_bStop || m_bAbort)
    return ret;

  if((m_cached_size + pkt->size) < maxDataSize)
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

bool OMXPlayerVideo::OpenDecoder()
{
	if (m_hints.fpsrate && m_hints.fpsscale)
	{
		m_fps = DVD_TIME_BASE / OMXReader::NormalizeFrameduration((double)DVD_TIME_BASE * m_hints.fpsscale / m_hints.fpsrate);
	}else
	{
		m_fps = 25;
	}

	if( m_fps > 100 || m_fps < 5 )
	{
		ofLog(OF_LOG_VERBOSE, "OMXPlayerVideo::OpenDecoder: Invalid framerate %d, using forced 25fps and just trust timestamps\n", (int)m_fps);
		m_fps = 25;
	}

	m_frametime = (double)DVD_TIME_BASE / m_fps;

	m_decoder = new COMXVideo();
	if(!m_decoder->Open(m_hints, m_av_clock, eglImage))
	{
		CloseDecoder();
		return false;
	}
	else
	{
		if (doDebugging) 
		{
			m_decoder->doDebugging = true;
		}
	  
		ofLog(OF_LOG_VERBOSE, "Video codec %s width %d height %d profile %d fps %f\n",
			  m_decoder->GetDecoderName().c_str() , m_hints.width, m_hints.height, m_hints.profile, m_fps);
	}

	if(m_av_clock)
	{
		m_av_clock->SetRefreshRate(m_fps);
	}
	
	return true;
}

bool OMXPlayerVideo::CloseDecoder()
{
  if(m_decoder)
  {
	  delete m_decoder;
	  m_decoder = NULL;
  }
  
  return true;
}

int OMXPlayerVideo::GetDecoderBufferSize()
{
	if(m_decoder)
	{
		return m_decoder->GetInputBufferSize();
	}
	return 0;
}

int OMXPlayerVideo::GetDecoderFreeSpace()
{
	if(m_decoder)
	{
	  return m_decoder->GetFreeSpace();
	}
	return 0;
}

void OMXPlayerVideo::WaitCompletion()
{
	if(!m_decoder)
	{
		return;
	}

	while(true)
	{
		Lock();
		if(m_packets.empty())
		{
			ofLogVerbose() << "packets empty";
			UnLock();
			break;
		}
		UnLock();
		OMXClock::OMXSleep(50);
	}

	m_decoder->WaitCompletion();
}

void OMXPlayerVideo::SetSpeed(int speed)
{
	m_speed = speed;
}
