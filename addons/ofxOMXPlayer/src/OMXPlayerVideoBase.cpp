
#include "OMXPlayerVideoBase.h"

unsigned count_bits(int32_t value)
{
	unsigned bits = 0;
	for(;value;++bits)
		value &= value - 1;
	return bits;
}

OMXPlayerVideoBase::OMXPlayerVideoBase()
{
	m_decoder = NULL;
	m_pStream = NULL;
	pthread_cond_init(&m_packet_cond, NULL);
	pthread_mutex_init(&m_lock, NULL);
	pthread_mutex_init(&m_lock_decoder, NULL);
	m_history_valid_pts = 0;
	m_flush_requested = false;
	isExiting = false;
}

OMXPlayerVideoBase::~OMXPlayerVideoBase()
{
	
	ofLogVerbose(__func__) << "START";

	Close();
	
	pthread_cond_destroy(&m_packet_cond);
	pthread_mutex_destroy(&m_lock);
	pthread_mutex_destroy(&m_lock_decoder);
	ofLogVerbose(__func__) << "END";
}

void OMXPlayerVideoBase::SetSpeed(int speed)
{
	m_speed = speed;
}

int OMXPlayerVideoBase::GetSpeed()
{
	return m_speed;
}

int  OMXPlayerVideoBase::GetDecoderFreeSpace()
{
	if(m_decoder)
	{
		return m_decoder->GetFreeSpace();
	}
	return 0;
}

int  OMXPlayerVideoBase::GetDecoderBufferSize()
{
	if(m_decoder)
	{
		return m_decoder->GetInputBufferSize();
	}
	return 0;
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
	m_history_valid_pts = (m_history_valid_pts << 1) | (pkt->pts != DVD_NOPTS_VALUE);
    double pts = pkt->pts;
    if(pkt->pts == DVD_NOPTS_VALUE && (m_iCurrentPts == DVD_NOPTS_VALUE || count_bits(m_history_valid_pts & 0xffff) < 4))
		pts = pkt->dts;
	
    if (pts != DVD_NOPTS_VALUE)
	{
		pts += m_iVideoDelay;
	}
	
    if(pts != DVD_NOPTS_VALUE)
	{
		m_iCurrentPts = pts;
	}
	
    while((int) m_decoder->GetFreeSpace() < pkt->size)
    {
		OMXClock::OMXSleep(10);
		if(m_flush_requested) return true;
    }
	
   // CLog::Log(LOGINFO, "CDVDPlayerVideo::Decode dts:%.0f pts:%.0f cur:%.0f, size:%d", pkt->dts, pkt->pts, m_iCurrentPts, pkt->size);
	//ofLog(OF_LOG_VERBOSE, "OMXPlayerVideoBase::Decode dts:%.0f pts:%.0f cur:%.0f, size:%d", pkt->dts, pkt->pts, m_iCurrentPts, pkt->size);
    m_decoder->Decode(pkt->data, pkt->size, pts);
}
	


void OMXPlayerVideoBase::Flush()
{
	ofLogVerbose(__func__) << "OMXPlayerVideoBase::Flush start";
	
	
	m_flush_requested = true;
	Lock();
	LockDecoder();
	 m_flush_requested = false;
	m_flush = true;
	while (!m_packets.empty())
	{
		OMXPacket *pkt = m_packets.front(); 
		m_packets.pop_front();
		//ofLogVerbose(__func__) << "OMXPlayerVideoBase->OMXReader FreePacket";
		OMXReader::FreePacket(pkt);
		
	}
	
	m_iCurrentPts = DVD_NOPTS_VALUE;
	m_cached_size = 0;
	
	if(m_decoder)
	{
		ofLogVerbose(__func__) << "OMXPlayerVideoBase::m_decoder->Reset";
		//m_decoder->Reset();
	}
	
	UnLockDecoder();
	UnLock();
	ofLogVerbose(__func__) << "OMXPlayerVideoBase::Flush end";
}



bool OMXPlayerVideoBase::AddPacket(OMXPacket *pkt)
{
	bool ret = false;
	
	if(!pkt)
	{
		return ret;
	}
	
	if(m_bStop || m_bAbort)
	{
		return ret;
	}
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


void OMXPlayerVideoBase::Process()
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

bool OMXPlayerVideoBase::CloseDecoder()
{
	if(m_decoder)
	{
		delete m_decoder;
	}
	m_decoder   = NULL;
	return true;
}

void OMXPlayerVideoBase::WaitCompletion()
{
	ofLogVerbose(__func__) << "OMXPlayerVideoBase WaitCompletion";
	
	if(!m_decoder)
	{
		return; 
	}
	
	while(true)
	{
		Lock();
		if(m_packets.empty())
		{
			UnLock();
			break;
		}
		UnLock();
		OMXClock::OMXSleep(50);
	}
	
	m_decoder->WaitCompletion();
}

bool OMXPlayerVideoBase::Close()
{
	ofLogVerbose(__func__) << " START";
	m_bAbort  = true;
	m_flush   = true;
	
	ofLogVerbose(__func__) << "isExiting: " << isExiting;
	if (!isExiting) 
	{
		Flush();
	}
	
	
	if(ThreadHandle())
	{
		Lock();
		pthread_cond_broadcast(&m_packet_cond);
		UnLock();
		
		StopThread();
	}
	//ofLogVerbose(__func__) << "OMXPlayerVideoBase::Close() pre CloseDecoder";
	//CloseDecoder();
	
	m_dllAvUtil.Unload();
	m_dllAvCodec.Unload();
	m_dllAvFormat.Unload();
	
	m_open          = false;
	m_stream_id     = -1;
	m_iCurrentPts   = DVD_NOPTS_VALUE;
	m_pStream       = NULL;
	m_pts           = 0;
	m_speed         = DVD_PLAYSPEED_NORMAL;
	
	ofLogVerbose(__func__) << " END";
	return true;
}
