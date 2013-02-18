#pragma once
#include "ofMain.h"

#include "DllAvUtil.h"
#include "DllAvFormat.h"
#include "DllAvCodec.h"

#include "OMXReader.h"
#include "OMXClock.h"
#include "OMXStreamInfo.h"
#include "OMXVideo.h"
#include "OMXThread.h"

#include <deque>
#include <sys/types.h>

using namespace std;

class OMXPlayerVideo : public OMXThread
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
	pthread_cond_t            m_picture_cond;
	pthread_mutex_t           m_lock;
	pthread_mutex_t           m_lock_decoder;
	OMXClock                  *m_av_clock;
	COMXVideo                 *m_decoder;
	float                     m_fps;
	double                    m_frametime;
	bool                      m_bAbort;
	bool                      m_flush;
	unsigned int              m_cached_size;
	double                    m_iVideoDelay;
	double                    m_pts;
	bool                      m_syncclock;
	int                       m_speed;
	double                    m_FlipTimeStamp; // time stamp of last flippage. used to play at a forced framerate

	void Lock();
	void UnLock();
	void LockDecoder();
	void UnLockDecoder();

private:
public:
	OMXPlayerVideo();
	~OMXPlayerVideo();
	bool Open(COMXStreamInfo &hints, OMXClock *av_clock, EGLImageKHR eglImage_);
	bool Close();
	void Output(double pts);
	bool Decode(OMXPacket *pkt);
	void Process();
	void Flush();
	bool AddPacket(OMXPacket *pkt);
	bool OpenDecoder();
	bool CloseDecoder();
	int  GetDecoderBufferSize();
	int  GetDecoderFreeSpace();
	double GetCurrentPTS() { return m_pts; };
	double GetFPS() { return m_fps; };
	unsigned int GetCached() { return m_cached_size; };
	void  WaitCompletion();
	void SetSpeed(int iSpeed);
	EGLImageKHR eglImage;
	int maxDataSize;
};

