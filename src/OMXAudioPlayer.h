#pragma once
#include "ofMain.h"


#include "LIBAV_INCLUDES.h"


#include "PCMRemap.h"

#include "OMXReader.h"
#include "OMXClock.h"
#include "StreamInfo.h"
#include "OMXAudioDecoder.h"


#include "AudioCodecOMX.h"

#include "OMXThread.h"

#include <deque>
#include <string>
#include <sys/types.h>


class OMXAudioPlayer : public OMXThread
{
	protected:
		std::deque<OMXPacket*> packets;
		StreamInfo  omxStreamInfo;
		double      currentPTS;
    
		pthread_cond_t  m_packet_cond;
		pthread_cond_t  m_audio_cond;
		pthread_mutex_t m_lock;
		pthread_mutex_t m_lock_decoder;
		OMXClock*           omxClock;
		OMXReader*          omxReader;
    
		OMXAudioDecoder*    decoder;
		string         codecName;
		string         deviceName;
    
		bool    doPassthrough;
		bool    doHardwareDecode;
		bool    doBoostOnDownmix;
		bool    doAbort;
		bool    doFlush;
    
		enum PCMChannels*   channelMap;
		unsigned int        cachedSize;
		AudioCodecOMX       *audioCodecOMX;
    
		int     speed;
		bool    hasErrors;
        bool    isOpen;


		void lock();
		void unlock();
		void lockDecoder();
		void unlockDecoder();
	private:
	public:
		OMXAudioPlayer();
		~OMXAudioPlayer();
		bool open(StreamInfo& hints,
                  OMXClock *av_clock,
                  OMXReader *omx_reader,
		          std::string device);
    
		bool close();
		bool decode(OMXPacket *pkt);
		void process();
		void flush();
		bool addPacket(OMXPacket *pkt);
		bool openCodec();
		void closeCodec();
		OMXAudioDecoder::EEncoded processPassthrough(StreamInfo hints);
		bool openDecoder();
		bool closeDecoder();
		double getCurrentPTS()
		{
			return currentPTS;
		};
		void WaitCompletion();
		void submitEOS();
		bool EOS();

		unsigned int getCached()
		{
			lock();
			unsigned int cached_size = cachedSize;
			unlock();
			return cached_size;

		};

		void setCurrentVolume(long nVolume);
		long getCurrentVolume();
		void setSpeed(int iSpeed);
		bool getError()
		{
            return hasErrors;
		};
};
