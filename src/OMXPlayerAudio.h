#pragma once
#include "ofMain.h"


#include "LIBAV_INCLUDES.h"


#include "PCMRemap.h"

#include "OMXReader.h"
#include "OMXClock.h"
#include "OMXStreamInfo.h"
#include "OMXAudio.h"


#include "AudioCodecOMX.h"

#include "OMXThread.h"

#include <deque>
#include <string>
#include <sys/types.h>


class OMXPlayerAudio : public OMXThread
{
	protected:
		int                       streamID;
		std::deque<OMXPacket *>   packets;
		bool                      isOpen;
		OMXStreamInfo            omxStreamInfo;
		double                    currentPTS;
		pthread_cond_t            m_packet_cond;
		pthread_cond_t            m_audio_cond;
		pthread_mutex_t           m_lock;
		pthread_mutex_t           m_lock_decoder;
		OMXClock*					omxClock;
		OMXReader*					omxReader;
		OMXAudio                 *decoder;
		std::string               codecName;
		std::string               deviceName;
		bool                      doPassthrough;
		bool                      doHardwareDecode;
		OMXAudio::EEncoded  m_passthrough;
		bool                      doBoostOnDownmix;
		bool                      doAbort;
		bool                      doFlush;
		enum PCMChannels          *channelMap;
		unsigned int              cachedSize;
		AudioCodecOMX         *audioCodecOMX;
		int                       speed;

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
		bool Open(OMXStreamInfo& hints,
                  OMXClock *av_clock,
                  OMXReader *omx_reader,
		          std::string device);
    
		bool Close();
		bool decode(OMXPacket *pkt);
		void Process();
		void Flush();
		bool addPacket(OMXPacket *pkt);
		bool OpenAudioCodec();
		void CloseAudioCodec();
		OMXAudio::EEncoded IsPassthrough(OMXStreamInfo hints);
		bool openDecoder();
		bool closeDecoder();
		//double GetDelay();
		//double GetCacheTime();
		double getCurrentPTS()
		{
			return currentPTS;
		};
		void WaitCompletion();
		void submitEOS();
		bool EOS();

		unsigned int GetCached()
		{
			Lock();
			unsigned int cached_size = cachedSize;
			UnLock();
			return cached_size;

		};

		void setCurrentVolume(long nVolume);
		long getCurrentVolume();
		void setSpeed(int iSpeed);
		bool Error()
		{
			return !m_player_error;
		};
};
