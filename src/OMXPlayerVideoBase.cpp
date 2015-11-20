
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
	isOpen          = false;
	streamID     = -1;
	omxClock      = NULL;
	decoder       = NULL;
	m_fps           = 25.0f;
	doFlush         = false;
	cachedSize   = 0;
	m_iVideoDelay   = 0;
	currentPTS	= DVD_NOPTS_VALUE;
	speed         = DVD_PLAYSPEED_NORMAL;

	decoder = NULL;
	pthread_cond_init(&m_packet_cond, NULL);
	pthread_mutex_init(&m_lock, NULL);
	pthread_mutex_init(&m_lock_decoder, NULL);
	validHistoryPTS = 0;
	doFlush_requested = false;
	isExiting = false;
}


double OMXPlayerVideoBase::getCurrentPTS()
{
	return currentPTS;
}

double OMXPlayerVideoBase::GetFPS()
{
	return m_fps;
}

unsigned int OMXPlayerVideoBase::getCached()
{
	return cachedSize;
}


int OMXPlayerVideoBase::getCurrentFrame()
{
	if (decoder) 
	{
		return decoder->getCurrentFrame();
	}
	return 0;
}

void OMXPlayerVideoBase::resetFrameCounter()
{
	if (decoder) 
	{
		decoder->resetFrameCounter();
	}
}


void OMXPlayerVideoBase::setSpeed(int speed_)
{
	speed = speed_;
}

int OMXPlayerVideoBase::getSpeed()
{
	return speed;
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

bool OMXPlayerVideoBase::decode(OMXPacket *pkt)
{
	if(!pkt)
	{
		return false;
	}
	
	validHistoryPTS = (validHistoryPTS << 1) | (pkt->pts != DVD_NOPTS_VALUE);
	double pts = pkt->pts;
	if(pkt->pts == DVD_NOPTS_VALUE && (currentPTS == DVD_NOPTS_VALUE || count_bits(validHistoryPTS & 0xffff) < 4))
	{
		pts = pkt->dts;
	}
	
	if (pts != DVD_NOPTS_VALUE)
	{
		pts += m_iVideoDelay;
	}

	if(pts != DVD_NOPTS_VALUE)
	{
		currentPTS = pts;
	}
	
	// CLog::Log(LOGINFO, "CDVDPlayerVideo::Decode dts:%.0f pts:%.0f cur:%.0f, size:%d", pkt->dts, pkt->pts, currentPTS, pkt->size);
	//ofLog(OF_LOG_VERBOSE, "OMXPlayerVideoBase::Decode dts:%.0f pts:%.0f cur:%.0f, size:%d", pkt->dts, pkt->pts, currentPTS, pkt->size);
	return decoder->decode(pkt->data, pkt->size, pts);
}



void OMXPlayerVideoBase::flush()
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

	currentPTS = DVD_NOPTS_VALUE;
	cachedSize = 0;

	if(decoder)
	{
		//ofLogVerbose(__func__) << "OMXPlayerVideoBase::decoder->Reset";
		decoder->Reset();
	}

	UnLockDecoder();
	UnLock();
	//ofLogVerbose(__func__) << "OMXPlayerVideoBase::Flush end";
}



bool OMXPlayerVideoBase::addPacket(OMXPacket *pkt)
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

	if((cachedSize + pkt->size) < MAX_DATA_SIZE)
	{
		Lock();
			cachedSize += pkt->size;
			packets.push_back(pkt);
		UnLock();
		ret = true;
		pthread_cond_broadcast(&m_packet_cond);
	}

	return ret;
}




void OMXPlayerVideoBase::Process()
{
	OMXPacket *omxPacket = NULL;

	//m_pts was previously set to 0 - might need later...
	//currentPTS = 0;
	
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
		if(doFlush && omxPacket)
		{
			OMXReader::FreePacket(omxPacket);
			omxPacket = NULL;
			doFlush = false;
		}
		else
		{
			if(!omxPacket && !packets.empty())
			{
				omxPacket = packets.front();
				cachedSize -= omxPacket->size;
                //ofLogNotice() << "omxPacket->pts: " << omxPacket->pts;
				packets.pop_front();
			}
		}
		UnLock();

		LockDecoder();
		if(doFlush && omxPacket)
		{
			OMXReader::FreePacket(omxPacket);
			omxPacket = NULL;
			doFlush = false;
		}
		else
		{
			if(omxPacket && decode(omxPacket))
			{
				
				OMXReader::FreePacket(omxPacket);
				omxPacket = NULL;
			}
		}
		
		UnLockDecoder();


	}
	
	if(omxPacket)
	{
		OMXReader::FreePacket(omxPacket);
	}
}

bool OMXPlayerVideoBase::closeDecoder()
{
	if(decoder)
	{
		delete decoder;
	}
	decoder   = NULL;
	return true;
}

void OMXPlayerVideoBase::submitEOS()
{
	if(decoder)
	{
		decoder->submitEOS();
	}
}

bool OMXPlayerVideoBase::EOS()
{
	bool atEndofStream = false;

	if (decoder)
	{
		if (packets.empty() && decoder->EOS())
		{

			atEndofStream = true;
			//ofLogVerbose(__func__) << "packets.empty() && decoder->EOS(): " << atEndofStream;
		}
	}
	return atEndofStream;
}

