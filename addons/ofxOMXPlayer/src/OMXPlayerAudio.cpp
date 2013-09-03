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



#include "linux/XMemUtils.h"


#define MAX_DATA_SIZE    3 * 1024 * 1024

OMXPlayerAudio::OMXPlayerAudio()
{
  m_open          = false;
  m_stream_id     = -1;
  m_pStream       = NULL;
  m_av_clock      = NULL;
  m_omx_reader    = NULL;
  m_decoder       = NULL;
  m_flush         = false;
  m_cached_size   = 0;
  m_pChannelMap   = NULL;
  m_pAudioCodec   = NULL;
  m_speed         = DVD_PLAYSPEED_NORMAL;
  m_player_error  = true;

  pthread_cond_init(&m_packet_cond, NULL);
  pthread_cond_init(&m_audio_cond, NULL);
  pthread_mutex_init(&m_lock, NULL);
  pthread_mutex_init(&m_lock_decoder, NULL);
}

OMXPlayerAudio::~OMXPlayerAudio()
{
	if(m_open)
	{
		Close();
	}


	pthread_cond_destroy(&m_audio_cond);
	pthread_cond_destroy(&m_packet_cond);
	pthread_mutex_destroy(&m_lock);
	pthread_mutex_destroy(&m_lock_decoder);
}

void OMXPlayerAudio::Lock()
{
  if(m_use_thread)
    pthread_mutex_lock(&m_lock);
}

void OMXPlayerAudio::UnLock()
{
  if(m_use_thread)
    pthread_mutex_unlock(&m_lock);
}

void OMXPlayerAudio::LockDecoder()
{
  if(m_use_thread)
    pthread_mutex_lock(&m_lock_decoder);
}

void OMXPlayerAudio::UnLockDecoder()
{
  if(m_use_thread)
    pthread_mutex_unlock(&m_lock_decoder);
}

bool OMXPlayerAudio::Open(COMXStreamInfo &hints, OMXClock *av_clock, OMXReader *omx_reader,
                          std::string device, bool passthrough, bool hw_decode,
                          bool boost_on_downmix, bool use_thread)
{
  if(ThreadHandle())
    Close();

  if (!m_dllAvUtil.Load() || !m_dllAvCodec.Load() || !m_dllAvFormat.Load() || !av_clock)
    return false;
  
  m_dllAvFormat.av_register_all();

  m_hints       = hints;
  m_av_clock    = av_clock;
  m_omx_reader  = omx_reader;
  m_device      = device;
  m_passthrough = IAudioRenderer::ENCODED_NONE;
  m_hw_decode   = false;
  m_use_passthrough = passthrough;
  m_use_hw_decode   = hw_decode;
  m_boost_on_downmix = boost_on_downmix;
  m_iCurrentPts = DVD_NOPTS_VALUE;
  m_bAbort      = false;
  m_use_thread  = use_thread;
  m_flush       = false;
  m_cached_size = 0;
  m_pAudioCodec = NULL;
  m_pChannelMap = NULL;
  m_speed       = DVD_PLAYSPEED_NORMAL;

 // m_av_clock->SetMasterClock(false);

  m_player_error = OpenAudioCodec();
  if(!m_player_error)
  {
    Close();
    return false;
  }

  m_player_error = OpenDecoder();
  if(!m_player_error)
  {
    Close();
    return false;
  }

  if(m_use_thread)
    Create();

  m_open        = true;

  return true;
}

bool OMXPlayerAudio::Close()
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
  CloseAudioCodec();

  m_open          = false;
  m_stream_id     = -1;
  m_iCurrentPts   = DVD_NOPTS_VALUE;
  m_pStream       = NULL;
  m_speed         = DVD_PLAYSPEED_NORMAL;

  m_dllAvUtil.Unload();
  m_dllAvCodec.Unload();
  m_dllAvFormat.Unload();

  return true;
}



bool OMXPlayerAudio::Decode(OMXPacket *pkt)
{
  if(!pkt)
    return false;

  /* last decoder reinit went wrong */
  if(!m_decoder || !m_pAudioCodec)
    return true;

  if(!m_omx_reader->IsActive(OMXSTREAM_AUDIO, pkt->stream_index))
    return true; 

  int channels = pkt->hints.channels;

  /* 6 channel have to be mapped to 8 for PCM */
  if(!m_passthrough && !m_hw_decode)
  {
    if(channels == 6)
      channels = 8;
  }
 
  unsigned int old_bitrate = m_hints.bitrate;
  unsigned int new_bitrate = pkt->hints.bitrate;

  /* only check bitrate changes on CODEC_ID_DTS, CODEC_ID_AC3, CODEC_ID_EAC3 */
  if(m_hints.codec != CODEC_ID_DTS && m_hints.codec != CODEC_ID_AC3 && m_hints.codec != CODEC_ID_EAC3)
  {
    new_bitrate = old_bitrate = 0;
  }

  /* audio codec changed. reinit device and decoder */
  if(m_hints.codec         != pkt->hints.codec ||
     m_hints.channels      != channels ||
     m_hints.samplerate    != pkt->hints.samplerate ||
     old_bitrate           != new_bitrate ||
     m_hints.bitspersample != pkt->hints.bitspersample)
  {
    ofLog(OF_LOG_VERBOSE, "C : %d %d %d %d %d\n", m_hints.codec, m_hints.channels, m_hints.samplerate, m_hints.bitrate, m_hints.bitspersample);
     ofLog(OF_LOG_VERBOSE, "N : %d %d %d %d %d\n", pkt->hints.codec, channels, pkt->hints.samplerate, pkt->hints.bitrate, pkt->hints.bitspersample);


    CloseDecoder();
    CloseAudioCodec();

    m_hints = pkt->hints;

    m_player_error = OpenAudioCodec();
    if(!m_player_error)
      return false;

    m_player_error = OpenDecoder();
    if(!m_player_error)
      return false;

    m_av_clock->OMXStateExecute();
    m_av_clock->OMXReset();
    m_av_clock->OMXResume();

  }

  if(!((int)m_decoder->GetSpace() > pkt->size))
    OMXClock::OMXSleep(10);

  if((int)m_decoder->GetSpace() > pkt->size)
  {
	  if(pkt->pts != DVD_NOPTS_VALUE)
		  m_iCurrentPts = pkt->pts;
	  else if(pkt->dts != DVD_NOPTS_VALUE)
		  m_iCurrentPts = pkt->dts;

    const uint8_t *data_dec = pkt->data;
    int            data_len = pkt->size;

    if(!m_passthrough && !m_hw_decode)
    {
      while(data_len > 0)
      {
        int len = m_pAudioCodec->Decode((BYTE *)data_dec, data_len);
        if( (len < 0) || (len >  data_len) )
        {
          m_pAudioCodec->Reset();
          break;
        }

        data_dec+= len;
        data_len -= len;

        uint8_t *decoded;
        int decoded_size = m_pAudioCodec->GetData(&decoded);

        if(decoded_size <=0)
          continue;

        int ret = 0;

        ret = m_decoder->AddPackets(decoded, decoded_size, pkt->dts, pkt->pts);

        if(ret != decoded_size)
        {
          ofLog(OF_LOG_VERBOSE, "error ret %d decoded_size %d\n", ret, decoded_size);
        }
      }
    }
    else
    {
		m_decoder->AddPackets(pkt->data, pkt->size, pkt->dts, pkt->pts);
    }
    return true;
  }
  else
  {
    return false;
  }
}

void OMXPlayerAudio::Process()
{
  OMXPacket *omx_pkt = NULL;

  while(!m_bStop && !m_bAbort)
  {
    Lock();
    if(m_packets.empty())
      pthread_cond_wait(&m_packet_cond, &m_lock);
    UnLock();

    if(m_bAbort)
      break;

    Lock();
    if(m_flush && omx_pkt)
    {
      OMXReader::FreePacket(omx_pkt);
      omx_pkt = NULL;
      m_flush = false;
    }
    else if(!omx_pkt && !m_packets.empty())
    {
      omx_pkt = m_packets.front();
      m_cached_size -= omx_pkt->size;
      m_packets.pop_front();
    }
    UnLock();
    
    LockDecoder();
    if(m_flush && omx_pkt)
    {
      OMXReader::FreePacket(omx_pkt);
      omx_pkt = NULL;
      m_flush = false;
    }
    else if(omx_pkt && Decode(omx_pkt))
    {
      OMXReader::FreePacket(omx_pkt);
      omx_pkt = NULL;
    }
    UnLockDecoder();
  }

  if(omx_pkt)
    OMXReader::FreePacket(omx_pkt);
}

void OMXPlayerAudio::Flush()
{
  ofLogVerbose() << "OMXPlayerAudio::Flush start";
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
    m_decoder->Flush();
  UnLockDecoder();
  UnLock();
	ofLogVerbose() << "OMXPlayerAudio::Flush end";
}

bool OMXPlayerAudio::AddPacket(OMXPacket *pkt)
{
  bool ret = false;

  if(!pkt)
    return ret;

  if(m_bStop || m_bAbort)
    return ret;

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

bool OMXPlayerAudio::OpenAudioCodec()
{
  m_pAudioCodec = new COMXAudioCodecOMX();

  if(!m_pAudioCodec->Open(m_hints))
  {
    delete m_pAudioCodec; m_pAudioCodec = NULL;
    return false;
  }

  m_pChannelMap = m_pAudioCodec->GetChannelMap();
  return true;
}

void OMXPlayerAudio::CloseAudioCodec()
{
  if(m_pAudioCodec)
    delete m_pAudioCodec;
  m_pAudioCodec = NULL;
}

IAudioRenderer::EEncoded OMXPlayerAudio::IsPassthrough(COMXStreamInfo hints)
{
#ifndef STANDALONE
  int  m_outputmode = 0;
  bool bitstream = false;
  IAudioRenderer::EEncoded passthrough = IAudioRenderer::ENCODED_NONE;

  m_outputmode = g_guiSettings.GetInt("audiooutput.mode");

  switch(m_outputmode)
  {
    case 0:
      passthrough = IAudioRenderer::ENCODED_NONE;
      break;
    case 1:
      bitstream = true;
      break;
    case 2:
      bitstream = true;
      break;
  }

  if(bitstream)
  {
    if(hints.codec == CODEC_ID_AC3 && g_guiSettings.GetBool("audiooutput.ac3passthrough"))
    {
      passthrough = IAudioRenderer::ENCODED_IEC61937_AC3;
    }
    if(hints.codec == CODEC_ID_DTS && g_guiSettings.GetBool("audiooutput.dtspassthrough"))
    {
      passthrough = IAudioRenderer::ENCODED_IEC61937_DTS;
    }
  }

  return passthrough;
#else
  if(m_device == "omx:local")
    return IAudioRenderer::ENCODED_NONE;

  IAudioRenderer::EEncoded passthrough = IAudioRenderer::ENCODED_NONE;

  if(hints.codec == CODEC_ID_AC3)
  {
    passthrough = IAudioRenderer::ENCODED_IEC61937_AC3;
  }
  if(hints.codec == CODEC_ID_EAC3)
  {
    passthrough = IAudioRenderer::ENCODED_IEC61937_EAC3;
  }
  if(hints.codec == CODEC_ID_DTS)
  {
    passthrough = IAudioRenderer::ENCODED_IEC61937_DTS;
  }

  return passthrough;
#endif
}

bool OMXPlayerAudio::OpenDecoder()
{
  bool bAudioRenderOpen = false;

  m_decoder = new COMXAudio();
  m_decoder->SetClock(m_av_clock);

  if(m_use_passthrough)
    m_passthrough = IsPassthrough(m_hints);

  if(!m_passthrough && m_use_hw_decode)
    m_hw_decode = COMXAudio::HWDecode(m_hints.codec);

  if(m_passthrough || m_use_hw_decode)
  {
    if(m_passthrough)
      m_hw_decode = false;
    bAudioRenderOpen = m_decoder->Initialize(m_device.substr(4), m_pChannelMap,
                                             m_hints, m_av_clock, m_passthrough, 
                                             m_hw_decode, m_boost_on_downmix);
  }
  else
  {
    unsigned int downmix_channels = m_hints.channels;

    /* omx needs 6 channels packed into 8 for PCM */
    if(m_hints.channels == 6)
      m_hints.channels = 8;

    bAudioRenderOpen = m_decoder->Initialize(m_device.substr(4), m_hints.channels, m_pChannelMap,
                                             downmix_channels, m_hints.samplerate, m_hints.bitspersample,
                                             false, m_boost_on_downmix, false, m_passthrough);
  }

  m_codec_name = m_omx_reader->GetCodecName(OMXSTREAM_AUDIO);
  
  if(!bAudioRenderOpen)
  {
    delete m_decoder; 
    m_decoder = NULL;
    return false;
  }
  else
  {
    if(m_passthrough)
    {
      ofLog(OF_LOG_VERBOSE, "Audio codec %s channels %d samplerate %d bitspersample %d\n",
        m_codec_name.c_str(), 2, m_hints.samplerate, m_hints.bitspersample);
    }
    else
    {
      ofLog(OF_LOG_VERBOSE, "Audio codec %s channels %d samplerate %d bitspersample %d\n",
        m_codec_name.c_str(), m_hints.channels, m_hints.samplerate, m_hints.bitspersample);
    }
  }

  return true;
}

bool OMXPlayerAudio::CloseDecoder()
{
  if(m_decoder)
    delete m_decoder;
  m_decoder   = NULL;
  return true;
}

double OMXPlayerAudio::GetDelay()
{
  if(m_decoder)
    return m_decoder->GetDelay();
  else
    return 0;
}

double OMXPlayerAudio::GetCacheTime()
{
  if(m_decoder)
    return m_decoder->GetCacheTime();
  else
    return 0;
}

void OMXPlayerAudio::WaitCompletion()
{
ofLogVerbose() << "OMXPlayerAudio::WaitCompletion";
  if(!m_decoder)
    return;

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

void OMXPlayerAudio::SetCurrentVolume(long nVolume)
{
	ofLogVerbose(__func__) << "nVolume: " << nVolume;
	if(m_decoder) 
	{
		m_decoder->SetCurrentVolume(nVolume);
	}
}

long OMXPlayerAudio::GetCurrentVolume()
{
  if(m_decoder)
  {
    return m_decoder->GetCurrentVolume();
  }
  else
  {
    return 0;
  }
}

void OMXPlayerAudio::SetSpeed(int speed)
{
  m_speed = speed;
}

