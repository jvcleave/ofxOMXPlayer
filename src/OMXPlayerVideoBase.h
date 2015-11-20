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

		virtual bool				Close() = 0;
		bool						Decode(OMXPacket *pkt);
		void						Process();
		void						Flush();

		bool						addPacket(OMXPacket *pkt);

		virtual bool				OpenDecoder() =0;

		bool						closeDecoder();
		double						GetCurrentPTS();
		double						GetFPS();

		unsigned int				GetCached();
	
		void						submitEOS();
		bool						EOS();

		void						Lock();
		void						UnLock();
		void						LockDecoder();
		void						UnLockDecoder();


		uint32_t					validHistoryPTS;
		bool						doFlush_requested;

		bool						isExiting;
	
		int getCurrentFrame();
		void resetFrameCounter();
};