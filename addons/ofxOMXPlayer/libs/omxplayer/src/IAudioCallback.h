#pragma once

class IAudioCallback
{
public:
  IAudioCallback() {};
  virtual ~IAudioCallback() {};
  virtual void OnInitialize(int iChannels, int iSamplesPerSec, int iBitsPerSample) = 0;
  virtual void OnAudioData(const unsigned char* pAudioData, int iAudioDataLength) = 0;

};