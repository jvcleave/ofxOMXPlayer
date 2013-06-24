#pragma once
#include "ofMain.h"


#include "DllAvUtil.h"
#include "DllAvFormat.h"
#include "DllAvFilter.h"
#include "DllAvCodec.h"

#include "utils/PCMRemap.h"

#include "OMXReader.h"
#include "OMXClock.h"
#include "OMXStreamInfo.h"
#include "OMXAudio.h"


#include "OMXAudioCodecOMX.h"

#include "OMXThread.h"

#include <deque>
#include <string>
#include <sys/types.h>


class OMXPlayerAudio : public OMXThread
{
protected:
  AVStream                  *m_pStream;
  int                       m_stream_id;
  std::deque<OMXPacket *>   m_packets;
  DllAvUtil                 m_dllAvUtil;
  DllAvCodec                m_dllAvCodec;
  DllAvFormat               m_dllAvFormat;
  bool                      m_open;
  COMXStreamInfo            m_hints;
  double                    m_iCurrentPts;
  pthread_cond_t            m_packet_cond;
  pthread_cond_t            m_audio_cond;
  pthread_mutex_t           m_lock;
  pthread_mutex_t           m_lock_decoder;
  OMXClock                  *m_av_clock;
  OMXReader                 *m_omx_reader;
  COMXAudio                 *m_decoder;
  std::string               m_codec_name;
  std::string               m_device;
  bool                      m_use_passthrough;
  bool                      m_use_hw_decode;
  IAudioRenderer::EEncoded  m_passthrough;
  bool                      m_hw_decode;
  bool                      m_boost_on_downmix;
  bool                      m_bAbort;	
  bool                      m_use_thread; 
  bool                      m_flush;
  enum PCMChannels          *m_pChannelMap;
  unsigned int              m_cached_size;
  COMXAudioCodecOMX         *m_pAudioCodec;
  int                       m_speed;

  int64_t m_errortime; //timestamp of last time we measured


  bool   m_player_error;


  void Lock();
  void UnLock();
  void LockDecoder();
  void UnLockDecoder();
private:
public:
  OMXPlayerAudio();
  ~OMXPlayerAudio();
  bool Open(COMXStreamInfo &hints, OMXClock *av_clock, OMXReader *omx_reader,
            std::string device, bool passthrough, bool hw_decode,
            bool boost_on_downmix, bool use_thread);
  bool Close();
  bool Decode(OMXPacket *pkt);
  void Process();
  void Flush();
  bool AddPacket(OMXPacket *pkt);
  bool OpenAudioCodec();
  void CloseAudioCodec();      
  IAudioRenderer::EEncoded IsPassthrough(COMXStreamInfo hints);
  bool OpenDecoder();
  bool CloseDecoder();
  double GetDelay();
  double GetCacheTime();
  double GetCurrentPTS() { return m_iCurrentPts; };
  void WaitCompletion();
  unsigned int GetCached()
  {
    Lock();
    unsigned int cached_size = m_cached_size;
    UnLock();
    return cached_size;

  };

  void SetCurrentVolume(long nVolume);
  long GetCurrentVolume();
  void SetSpeed(int iSpeed);
  bool Error() { return !m_player_error; };
};
