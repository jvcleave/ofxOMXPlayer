#pragma once
#include "OMXVideoPlayer.h"


#include "OMXEGLImage.h"


#include <deque>
#include <sys/types.h>



class OMXEGLImagePlayer : public OMXVideoPlayer
{


	
	

	//char debugInfoBuffer [1024];
	


public:
	OMXEGLImagePlayer();
	~OMXEGLImagePlayer();
	bool Open(COMXStreamInfo &hints, OMXClock *av_clock, EGLImageKHR eglImage_);
	//void SetSpeed(int iSpeed);
	//int GetSpeed(){return m_speed;};
	EGLImageKHR eglImage;
	OMXDecoder*					m_decoder;
	OMXEGLImage*				eglImageDecoder;
	//string debugInfo;
	//bool doDebugging;
	
	
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
	pthread_cond_t            m_picture_cond;
	pthread_mutex_t           m_lock;
	pthread_mutex_t           m_lock_decoder;
	OMXClock                  *m_av_clock;
	float                     m_fps;
	double                    m_frametime;
	bool                      m_bAbort;
	bool                      m_flush;
	bool                      m_syncclock;
	int                       m_speed;
	double                    m_pts;
	double                    m_FlipTimeStamp; // time stamp of last flippage. used to play at a forced framerate
	double                    m_iVideoDelay;
	unsigned int              m_cached_size;
	bool Close();
	void Output(double pts);
	bool Decode(OMXPacket *pkt);
	void Process();
	void Flush();
	//void UnFlush();
	bool AddPacket(OMXPacket *pkt);
	bool OpenDecoder();
	bool CloseDecoder();
	int  GetDecoderBufferSize();
	int  GetDecoderFreeSpace();
	double GetCurrentPTS() { return m_pts; };
	double GetFPS() { return m_fps; };
	unsigned int GetCached() { return m_cached_size; };
	void  WaitCompletion();
	void Lock();
	void UnLock();
	void LockDecoder();
	void UnLockDecoder();
};

