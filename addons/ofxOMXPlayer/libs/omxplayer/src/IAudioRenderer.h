
#pragma once


#include "utils/StdString.h"
#include "utils/PCMRemap.h"

class IAudioRenderer
{
public:
  enum EEncoded {
    ENCODED_NONE = 0,
    ENCODED_IEC61937_AC3,
    ENCODED_IEC61937_EAC3,
    ENCODED_IEC61937_DTS,
    ENCODED_IEC61937_MPEG,
    ENCODED_IEC61937_UNKNOWN,
  };

  IAudioRenderer() {};
  virtual ~IAudioRenderer() {};
  virtual bool Initialize(const CStdString& device, int iChannels, enum PCMChannels *channelMap, unsigned int downmixChannels, unsigned int uiSamplesPerSec, unsigned int uiBitsPerSample, bool bResample, bool boostOnDownmix, bool bIsMusic=false, EEncoded encoded = ENCODED_NONE) = 0;
  virtual float GetDelay() = 0;
  virtual float GetCacheTime() = 0;
  virtual float GetCacheTotal() { return 1.0f; }

  virtual unsigned int AddPackets(const void* data, unsigned int len) = 0;
  virtual bool IsResampling() { return false;};
  virtual unsigned int GetSpace() = 0;
  virtual bool Deinitialize() = 0;
  virtual bool Pause() = 0;
  virtual bool Stop() = 0;
  virtual bool Resume() = 0;
  virtual unsigned int GetChunkLen() = 0;

  virtual long GetCurrentVolume() const = 0;
  virtual void Mute(bool bMute) = 0;
  virtual bool SetCurrentVolume(long nVolume) = 0;
  virtual float GetCurrentAttenuation() { return m_remap.GetCurrentAttenuation(); }
  virtual int SetPlaySpeed(int iSpeed) = 0;
  virtual void WaitCompletion() = 0;

protected:
  CPCMRemap m_remap;

private:
};

