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


#include "OMXPlayerAudio.h"

#include <stdio.h>
#include <unistd.h>



#include "XMemUtils.h"


#define MAX_DATA_SIZE    3 * 1024 * 1024

OMXPlayerAudio::OMXPlayerAudio()
{
	isOpen          = false;
	omxClock      = NULL;
	omxReader    = NULL;
	decoder       = NULL;
	doFlush         = false;
	cachedSize   = 0;
	channelMap   = NULL;
	audioCodecOMX   = NULL;
	speed         = DVD_PLAYSPEED_NORMAL;
	hasErrors  = true;

	pthread_cond_init(&m_packet_cond, NULL);
	pthread_cond_init(&m_audio_cond, NULL);
	pthread_mutex_init(&m_lock, NULL);
	pthread_mutex_init(&m_lock_decoder, NULL);
}

OMXPlayerAudio::~OMXPlayerAudio()
{
	if(isOpen)
	{
		close();
	}


	pthread_cond_destroy(&m_audio_cond);
	pthread_cond_destroy(&m_packet_cond);
	pthread_mutex_destroy(&m_lock);
	pthread_mutex_destroy(&m_lock_decoder);
}

void OMXPlayerAudio::lock()
{
	pthread_mutex_lock(&m_lock);
}

void OMXPlayerAudio::unlock()
{
	pthread_mutex_unlock(&m_lock);
}

void OMXPlayerAudio::lockDecoder()
{
	pthread_mutex_lock(&m_lock_decoder);
}

void OMXPlayerAudio::unlockDecoder()
{
	pthread_mutex_unlock(&m_lock_decoder);
}

bool OMXPlayerAudio::open(OMXStreamInfo& hints, 
                          OMXClock *av_clock, 
                          OMXReader *omx_reader,
                          std::string device)
{
	if(ThreadHandle())
	{
		close();
	}

	if (!av_clock)
	{
		return false;
	}


	omxStreamInfo       = hints;
	omxClock    = av_clock;
	omxReader  = omx_reader;
	deviceName      = device;
	currentPTS = DVD_NOPTS_VALUE;
	doAbort      = false;
	doFlush       = false;
	cachedSize = 0;
	audioCodecOMX = NULL;
	channelMap = NULL;
	speed       = DVD_PLAYSPEED_NORMAL;

// omxClock->SetMasterClock(false);

	hasErrors = openCodec();
	if(!hasErrors)
	{
		close();
		return false;
	}

	hasErrors = openDecoder();
	if(!hasErrors)
	{
		close();
		return false;
	}

	Create();

	isOpen        = true;

	return true;
}

bool OMXPlayerAudio::close()
{
	doAbort  = true;
	doFlush   = true;

	flush();

	if(ThreadHandle())
	{
		lock();
		pthread_cond_broadcast(&m_packet_cond);
		unlock();

		StopThread("OMXPlayerAudio");
	}

	closeDecoder();
	closeCodec();

	isOpen          = false;
	streamID     = -1;
	currentPTS   = DVD_NOPTS_VALUE;
	speed         = DVD_PLAYSPEED_NORMAL;


	return true;
}



bool OMXPlayerAudio::decode(OMXPacket *pkt)
{
	if(!pkt)
	{
		return false;
	}

	/* last decoder reinit went wrong */
	if(!decoder || !audioCodecOMX)
	{
		return true;
	}

	if(!omxReader->isActive(OMXSTREAM_AUDIO, pkt->stream_index))
	{
		return true;
	}

	int channels = pkt->hints.channels;

	/* 6 channel have to be mapped to 8 for PCM */
	if(!m_passthrough)
	{
		if(channels == 6)
		{
			channels = 8;
		}
	}

	unsigned int old_bitrate = omxStreamInfo.bitrate;
	unsigned int new_bitrate = pkt->hints.bitrate;

	/* only check bitrate changes on CODEC_ID_DTS, CODEC_ID_AC3, CODEC_ID_EAC3 */
	if(omxStreamInfo.codec != CODEC_ID_DTS && omxStreamInfo.codec != CODEC_ID_AC3 && omxStreamInfo.codec != CODEC_ID_EAC3)
	{
		new_bitrate = old_bitrate = 0;
	}

	/* audio codec changed. reinit device and decoder */
	if(omxStreamInfo.codec         != pkt->hints.codec ||
	        omxStreamInfo.channels      != channels ||
	        omxStreamInfo.samplerate    != pkt->hints.samplerate ||
	        old_bitrate           != new_bitrate ||
	        omxStreamInfo.bitspersample != pkt->hints.bitspersample)
	{
       
		closeDecoder();
		closeCodec();

		omxStreamInfo = pkt->hints;

		hasErrors = openCodec();
		if(!hasErrors)
		{
			return false;
		}

		hasErrors = openDecoder();
		if(!hasErrors)
		{
			return false;
		}
	}
    
#if 1
	if(!((int)decoder->GetSpace() > pkt->size))
	{
		omxClock->sleep(10);
	}

	if((int)decoder->GetSpace() > pkt->size)
	{
		if(pkt->pts != DVD_NOPTS_VALUE)
		{
			currentPTS = pkt->pts;
		}
		else if(pkt->dts != DVD_NOPTS_VALUE)
		{
			currentPTS = pkt->dts;
		}

		uint8_t *data_dec = pkt->data;
		int            data_len = pkt->size;

		if(!m_passthrough)
		{
			while(data_len > 0)
			{
				int len = audioCodecOMX->decode((BYTE *)data_dec, data_len);
				if( (len < 0) || (len >  data_len) )
				{
					audioCodecOMX->Reset();
					break;
				}

				data_dec+= len;
				data_len -= len;

				uint8_t *decoded;
				int decoded_size = audioCodecOMX->GetData(&decoded);

				if(decoded_size <=0)
				{
					continue;
				}

				int ret = 0;

				ret = decoder->addPackets(decoded, decoded_size, pkt->dts, pkt->pts);

				if(ret != decoded_size)
				{
					ofLogError(__func__) << "ret : " << ret << " NOT EQUAL TO " << decoded_size;
				}
			}
		}
		else
		{
			decoder->addPackets(pkt->data, pkt->size, pkt->dts, pkt->pts);
		}
		return true;
	}
	else
	{
		return false;
	}
#endif
}

void OMXPlayerAudio::process()
{
	OMXPacket *omxPacket = NULL;

	while(!doStop && !doAbort)
	{
		lock();
		if(packets.empty())
		{
			pthread_cond_wait(&m_packet_cond, &m_lock);
		}
		unlock();

		if(doAbort)
		{
			break;
		}

		lock();
		if(doFlush && omxPacket)
		{
			OMXReader::freePacket(omxPacket);
			omxPacket = NULL;
			doFlush = false;
		}
		else if(!omxPacket && !packets.empty())
		{
			omxPacket = packets.front();
			cachedSize -= omxPacket->size;
			packets.pop_front();
		}
		unlock();

		lockDecoder();
		if(doFlush && omxPacket)
		{
			OMXReader::freePacket(omxPacket);
			omxPacket = NULL;
			doFlush = false;
		}
		else if(omxPacket && decode(omxPacket))
		{
			OMXReader::freePacket(omxPacket);
			omxPacket = NULL;
		}
		unlockDecoder();
	}

	if(omxPacket)
	{
		OMXReader::freePacket(omxPacket);
	}
}

void OMXPlayerAudio::flush()
{
	//ofLogVerbose(__func__) << "OMXPlayerAudio::Flush start";
	lock();
	lockDecoder();
	doFlush = true;
	while (!packets.empty())
	{
		OMXPacket *pkt = packets.front();
		packets.pop_front();
		OMXReader::freePacket(pkt);
	}
	currentPTS = DVD_NOPTS_VALUE;
	cachedSize = 0;
	if(decoder)
	{
		decoder->flush();
	}
	unlockDecoder();
	unlock();
	//ofLogVerbose(__func__) << "OMXPlayerAudio::Flush end";
}

bool OMXPlayerAudio::addPacket(OMXPacket *pkt)
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
		lock();
		cachedSize += pkt->size;
		packets.push_back(pkt);
		unlock();
		ret = true;
		pthread_cond_broadcast(&m_packet_cond);
	}

	return ret;
}

bool OMXPlayerAudio::openCodec()
{
	audioCodecOMX = new AudioCodecOMX();

	if(!audioCodecOMX->open(omxStreamInfo))
	{
		delete audioCodecOMX;
		audioCodecOMX = NULL;
		return false;
	}

	channelMap = audioCodecOMX->GetChannelMap();
	return true;
}

void OMXPlayerAudio::closeCodec()
{
	if(audioCodecOMX)
	{
		delete audioCodecOMX;
	}
	audioCodecOMX = NULL;
}

OMXAudio::EEncoded OMXPlayerAudio::processPassthrough(OMXStreamInfo hints)
{
    if(deviceName == "omx:local")
    {
        return OMXAudio::ENCODED_NONE;
    }
    
    OMXAudio::EEncoded passthrough = OMXAudio::ENCODED_NONE;
    
    if(hints.codec == CODEC_ID_AC3)
    {
        passthrough = OMXAudio::ENCODED_IEC61937_AC3;
    }
    if(hints.codec == CODEC_ID_EAC3)
    {
        passthrough = OMXAudio::ENCODED_IEC61937_EAC3;
    }
    if(hints.codec == CODEC_ID_DTS)
    {
        passthrough = OMXAudio::ENCODED_IEC61937_DTS;
    }
    
    return passthrough;

}

bool OMXPlayerAudio::openDecoder()
{
	//ofLogVerbose(__func__) << "doHardwareDecode: " << doHardwareDecode;
	//ofLogVerbose(__func__) << "doPassthrough: " << doPassthrough;
	bool bAudioRenderOpen = false;

	decoder = new OMXAudio();
	decoder->setClock(omxClock);

	if(doPassthrough)
	{
		m_passthrough = processPassthrough(omxStreamInfo);
	}

	

	if(m_passthrough || doHardwareDecode)
	{
		
        stringstream ss;
        ss << deviceName.substr(4);
        string name = ss.str();
		bAudioRenderOpen = decoder->init(name, 
                                           channelMap,
                                           omxStreamInfo, 
                                           omxClock,
                                           m_passthrough, 
                                           doBoostOnDownmix);
	}
	
	codecName = omxReader->getCodecName(OMXSTREAM_AUDIO);

	if(!bAudioRenderOpen)
	{
		delete decoder;
		decoder = NULL;
		return false;
	}
	else
	{
		if(m_passthrough)
		{
			ofLog(OF_LOG_VERBOSE, "USING PASSTHROUGH, Audio codec %s channels %d samplerate %d bitspersample %d\n",
			      codecName.c_str(), 2, omxStreamInfo.samplerate, omxStreamInfo.bitspersample);
		}
		else
		{
			ofLog(OF_LOG_VERBOSE, "PASSTHROUGH DISABLED, Audio codec %s channels %d samplerate %d bitspersample %d\n",
			      codecName.c_str(), omxStreamInfo.channels, omxStreamInfo.samplerate, omxStreamInfo.bitspersample);
		}
	}
	return true;
}

bool OMXPlayerAudio::closeDecoder()
{
	if(decoder)
	{
		delete decoder;
	}
	decoder   = NULL;
	return true;
}

void OMXPlayerAudio::submitEOS()
{
	if(decoder)
	{
		decoder->submitEOS();
	}
}

bool OMXPlayerAudio::EOS()
{
	return packets.empty() && (!decoder || decoder->EOS());
}

void OMXPlayerAudio::WaitCompletion()
{
	//ofLogVerbose(__func__) << "OMXPlayerAudio::WaitCompletion";
	if(!decoder)
	{
		return;
	}

	unsigned int nTimeOut = 2.0f * 1000;
	while(nTimeOut)
	{
		if(EOS())
		{
			ofLog(OF_LOG_VERBOSE, "%s::%s - got eos\n", "OMXPlayerAudio", __func__);
			break;
		}

		if(nTimeOut == 0)
		{
			ofLog(OF_LOG_ERROR, "%s::%s - wait for eos timed out\n", "OMXPlayerAudio", __func__);
			break;
		}
		omxClock->sleep(50);
		nTimeOut -= 50;
	}
}

void OMXPlayerAudio::setCurrentVolume(long nVolume)
{
	//ofLogVerbose(__func__) << "nVolume: " << nVolume;
	if(decoder)
	{
		decoder->setCurrentVolume(nVolume);
	}
}

long OMXPlayerAudio::getCurrentVolume()
{
	if(decoder)
	{
		return decoder->getCurrentVolume();
	}
	else
	{
		return 0;
	}
}

void OMXPlayerAudio::setSpeed(int speed_)
{
	speed = speed_;
}

