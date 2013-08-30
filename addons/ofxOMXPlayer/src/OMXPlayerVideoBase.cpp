
#include "OMXPlayerVideoBase.h"

OMXPlayerVideoBase::OMXPlayerVideoBase()
{
	m_decoder = NULL;
	m_pStream = NULL;
	pthread_cond_init(&m_packet_cond, NULL);
	//pthread_cond_init(&m_picture_cond, NULL);
	pthread_mutex_init(&m_lock, NULL);
	pthread_mutex_init(&m_lock_decoder, NULL);
}

OMXPlayerVideoBase::~OMXPlayerVideoBase()
{
	Close();
	
	pthread_cond_destroy(&m_packet_cond);
	//pthread_cond_destroy(&m_picture_cond);
	pthread_mutex_destroy(&m_lock);
	pthread_mutex_destroy(&m_lock_decoder);
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
	
	bool ret = false;
	
	if(!((int)m_decoder->GetFreeSpace() > pkt->size))
	{
		OMXClock::OMXSleep(10);
	}
	
	if(pkt->pts != DVD_NOPTS_VALUE)
	{
		m_pts = pkt->pts;
	}
	
	if((int)m_decoder->GetFreeSpace() > pkt->size)
    {
		if(pkt->pts != DVD_NOPTS_VALUE)
		{
			m_iCurrentPts = pkt->pts;
		}
		else if(pkt->dts != DVD_NOPTS_VALUE)
		{
			m_iCurrentPts = pkt->dts;
		}
		bool doDecodeFrameDebugging  = false;
		if (doDecodeFrameDebugging) 
		{
			ofLog(OF_LOG_VERBOSE, "OMXPlayerVideoBase::Decode dts:%.0f pts:%.0f cur:%.0f, size:%d", pkt->dts, pkt->pts, m_iCurrentPts, pkt->size);

		}
		m_decoder->Decode(pkt->data, pkt->size, pkt->dts, pkt->pts);
		ret = true;
    }
    else
	{
		// video fifo is full, allow other tasks to run
		OMXClock::OMXSleep(10);
	}
	return ret;
}
	


void OMXPlayerVideoBase::Flush()
{
	ofLogVerbose() << "OMXPlayerVideoBase::Flush start";
	Lock();
	LockDecoder();
	m_flush = true;
	while (!m_packets.empty())
	{
		OMXPacket *pkt = m_packets.front(); 
		m_packets.pop_front();
		//ofLogVerbose() << "OMXPlayerVideoBase->OMXReader FreePacket";
		OMXReader::FreePacket(pkt);
		
	}
	
	m_iCurrentPts = DVD_NOPTS_VALUE;
	m_cached_size = 0;
	
	if(m_decoder)
	{
		ofLogVerbose() << "OMXPlayerVideoBase::m_decoder->Reset";
		m_decoder->Reset();
	}
	
	UnLockDecoder();
	UnLock();
	ofLogVerbose() << "OMXPlayerVideoBase::Flush end";
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
	ofLogVerbose() << "OMXPlayerVideoBase::Close()";
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
	ofLogVerbose() << "OMXPlayerVideoBase::Close() pre CloseDecoder";
	CloseDecoder();
	
	m_dllAvUtil.Unload();
	m_dllAvCodec.Unload();
	m_dllAvFormat.Unload();
	
	m_open          = false;
	m_stream_id     = -1;
	m_iCurrentPts   = DVD_NOPTS_VALUE;
	m_pStream       = NULL;
	m_pts           = 0;
	m_speed         = DVD_PLAYSPEED_NORMAL;
	
	return true;
}
