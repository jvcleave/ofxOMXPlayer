#pragma once
#include "ofMain.h"
#include "ofxOMXPlayerSettings.h"

#include "LIBAV_INCLUDES.h"


#include "OMXReader.h"
#include "OMXClock.h"
#include "OMXStreamInfo.h"
#include "OMXThread.h"
#include "OMXDecoderBase.h"

#define MAX_DATA_SIZE    10 * 1024 * 1024


class OMXPlayerVideoBase: public OMXThread
{
	public:
		OMXPlayerVideoBase();
		//~OMXPlayerVideoBase();
		OMXDecoderBase*				decoder;
		int							streamID;
		std::deque<OMXPacket *>		packets;

		bool						isOpen;
		OMXStreamInfo				omxStreamInfo;
		double						currentPTS;

		pthread_cond_t				m_packet_cond;
		//pthread_cond_t				m_picture_cond;
		pthread_mutex_t				m_lock;
		pthread_mutex_t				m_lock_decoder;

		OMXClock*					omxClock;
		float						m_fps;
		double						m_frametime;
		bool						doAbort;
		bool						doFlush;
		int							speed;
		double						m_FlipTimeStamp; // time stamp of last flippage. used to play at a forced framerate
		double						m_iVideoDelay;
		unsigned int				cachedSize;


		void						setSpeed(int speed);
		int							getSpeed();

		virtual bool				close() = 0;
		bool						decode(OMXPacket *pkt);
		void						process();
		void						flush();

		bool						addPacket(OMXPacket *pkt);

		virtual bool				openDecoder() =0;

		bool						closeDecoder();
		double						getCurrentPTS();
		double						GetFPS();

		unsigned int				getCached();
	
		void						submitEOS();
		bool						EOS();

		void						lock();
		void						unlock();
		void						lockDecoder();
		void						unlockDecoder();


		uint32_t					validHistoryPTS;
		bool						doFlush_requested;

		bool						isExiting;
	
		int getCurrentFrame();
		void resetFrameCounter();
};