
#include "OMXPlayerVideoBase.h"

unsigned count_bits(int32_t value)
{
	unsigned bits = 0;
	for(; value; ++bits)
	{
		value &= value - 1;
	}
	return bits;
}

OMXPlayerVideoBase::OMXPlayerVideoBase()
{
	m_open          = false;
	streamID     = -1;
	omxClock      = NULL;
	m_decoder       = NULL;
	m_fps           = 25.0f;
	doFlush         = false;
	m_cached_size   = 0;
	m_iVideoDelay   = 0;
	m_iCurrentPts	= DVD_NOPTS_VALUE;
	m_speed         = DVD_PLAYSPEED_NORMAL;

	m_decoder = NULL;
	pthread_cond_init(&m_packet_cond, NULL);
	pthread_mutex_init(&m_lock, NULL);
	pthread_mutex_init(&m_lock_decoder, NULL);
	validHistoryPTS = 0;
	doFlush_requested = false;
	isExiting = false;
}


double OMXPlayerVideoBase::GetCurrentPTS()
{
	return m_iCurrentPts;
}

double OMXPlayerVideoBase::GetFPS()
{
	return m_fps;
}

unsigned int OMXPlayerVideoBase::GetCached()
{
	return m_cached_size;
}


int OMXPlayerVideoBase::getCurrentFrame()
{
	if (m_decoder) 
	{
		return m_decoder->getCurrentFrame();
	}
	return 0;
}

void OMXPlayerVideoBase::resetFrameCounter()
{
	if (m_decoder) 
	{
		m_decoder->resetFrameCounter();
	}
}


void OMXPlayerVideoBase::setSpeed(int speed)
{
	m_speed = speed;
}

int OMXPlayerVideoBase::getSpeed()
{
	return m_speed;
}




void OMXPlayerVideoBase::Lock()
{
	pthread_mutex_lock(&m_lock);
}

void OMXPlayerVideoBase::UnLock()
{
	pthread_mutex_unlock(&m_lock);
}

void OMXPlayerVideoBase::LockDecoder()
{
	pthread_mutex_lock(&m_lock_decoder);
}

void OMXPlayerVideoBase::UnLockDecoder()
{
	pthread_mutex_unlock(&m_lock_decoder);
}

bool OMXPlayerVideoBase::Decode(OMXPacket *pkt)
{
	if(!pkt)
	{
		return false;
	}
	
	validHistoryPTS = (validHistoryPTS << 1) | (pkt->pts != DVD_NOPTS_VALUE);
	double pts = pkt->pts;
	if(pkt->pts == DVD_NOPTS_VALUE && (m_iCurrentPts == DVD_NOPTS_VALUE || count_bits(validHistoryPTS & 0xffff) < 4))
	{
		pts = pkt->dts;
	}
	
	if (pts != DVD_NOPTS_VALUE)
	{
		pts += m_iVideoDelay;
	}

	if(pts != DVD_NOPTS_VALUE)
	{
		m_iCurrentPts = pts;
	}
	
	// CLog::Log(LOGINFO, "CDVDPlayerVideo::Decode dts:%.0f pts:%.0f cur:%.0f, size:%d", pkt->dts, pkt->pts, m_iCurrentPts, pkt->size);
	//ofLog(OF_LOG_VERBOSE, "OMXPlayerVideoBase::Decode dts:%.0f pts:%.0f cur:%.0f, size:%d", pkt->dts, pkt->pts, m_iCurrentPts, pkt->size);
	return m_decoder->Decode(pkt->data, pkt->size, pts);
}



void OMXPlayerVideoBase::Flush()
{
	//ofLogVerbose(__func__) << "OMXPlayerVideoBase::Flush start";


	doFlush_requested = true;
	Lock();
	LockDecoder();
	doFlush_requested = false;
	doFlush = true;
	while (!packets.empty())
	{
		OMXPacket *pkt = packets.front();
		packets.pop_front();
		//ofLogVerbose(__func__) << "OMXPlayerVideoBase->OMXReader FreePacket";
		OMXReader::FreePacket(pkt);

	}

	m_iCurrentPts = DVD_NOPTS_VALUE;
	m_cached_size = 0;

	if(m_decoder)
	{
		//ofLogVerbose(__func__) << "OMXPlayerVideoBase::m_decoder->Reset";
		m_decoder->Reset();
	}

	UnLockDecoder();
	UnLock();
	//ofLogVerbose(__func__) << "OMXPlayerVideoBase::Flush end";
}



bool OMXPlayerVideoBase::AddPacket(OMXPacket *pkt)
{
	bool ret = false;

	if(!pkt)
	{
		return ret;
	}

	if(doStop || doAbort)
	{
		return ret;
	}

	if((m_cached_size + pkt->size) < MAX_DATA_SIZE)
	{
		Lock();
			m_cached_size += pkt->size;
			packets.push_back(pkt);
		UnLock();
		ret = true;
		pthread_cond_broadcast(&m_packet_cond);
	}

	return ret;
}




void OMXPlayerVideoBase::Process()
{
	OMXPacket *omx_pkt = NULL;

	//m_pts was previously set to 0 - might need later...
	//m_iCurrentPts = 0;
	
	while(!doStop && !doAbort)
	{
		Lock();
		if(packets.empty())
		{
			pthread_cond_wait(&m_packet_cond, &m_lock);
		}
		UnLock();

		if(doAbort)
		{
			break;
		}

		Lock();
		if(doFlush && omx_pkt)
		{
			OMXReader::FreePacket(omx_pkt);
			omx_pkt = NULL;
			doFlush = false;
		}
		else
		{
			if(!omx_pkt && !packets.empty())
			{
				omx_pkt = packets.front();
				m_cached_size -= omx_pkt->size;
                //ofLogNotice() << "omx_pkt->pts: " << omx_pkt->pts;
				packets.pop_front();
			}
		}
		UnLock();

		LockDecoder();
		if(doFlush && omx_pkt)
		{
			OMXReader::FreePacket(omx_pkt);
			omx_pkt = NULL;
			doFlush = false;
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

bool OMXPlayerVideoBase::CloseDecoder()
{
	if(m_decoder)
	{
		delete m_decoder;
	}
	m_decoder   = NULL;
	return true;
}

void OMXPlayerVideoBase::submitEOS()
{
	if(m_decoder)
	{
		m_decoder->submitEOS();
	}
}

bool OMXPlayerVideoBase::EOS()
{
	bool atEndofStream = false;

	if (m_decoder)
	{
		if (packets.empty() && m_decoder->EOS())
		{

			atEndofStream = true;
			//ofLogVerbose(__func__) << "packets.empty() && m_decoder->EOS(): " << atEndofStream;
		}
	}
	return atEndofStream;
}

