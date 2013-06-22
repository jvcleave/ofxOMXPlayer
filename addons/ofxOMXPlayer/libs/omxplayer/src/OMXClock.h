#pragma once

#include "DllAvFormat.h"

#include "OMXCore.h"

#define DVD_TIME_BASE 1000000
#define DVD_NOPTS_VALUE    (-1LL<<52) // should be possible to represent in both double and __int64

#define DVD_TIME_TO_SEC(x)  ((int)((double)(x) / DVD_TIME_BASE))
#define DVD_TIME_TO_MSEC(x) ((int)((double)(x) * 1000 / DVD_TIME_BASE))
#define DVD_SEC_TO_TIME(x)  ((double)(x) * DVD_TIME_BASE)
#define DVD_MSEC_TO_TIME(x) ((double)(x) * DVD_TIME_BASE / 1000)

#define DVD_PLAYSPEED_PAUSE       0       // frame stepping
#define DVD_PLAYSPEED_NORMAL      1000

static inline OMX_TICKS ToOMXTime(int64_t pts)
{
	OMX_TICKS ticks;
	ticks.nLowPart = pts;
	ticks.nHighPart = pts >> 32;
	return ticks;
}
static inline int64_t FromOMXTime(OMX_TICKS ticks)
{
	int64_t pts = ticks.nLowPart | ((uint64_t)(ticks.nHighPart) << 32);
	return pts;
}

class OMXClock
{
protected:
	bool              m_pause;
	bool              m_has_video;
	bool              m_has_audio;
	int               m_omx_speed;
	pthread_mutex_t   m_lock;
private:
	COMXCoreComponent m_omx_clock;
	DllAvFormat       m_dllAvFormat;
public:
	OMXClock();
	~OMXClock();
	void Lock();
	void UnLock();
	bool OMXInitialize(bool has_video, bool has_audio);
	void OMXDeinitialize();
	bool OMXIsPaused() { return m_pause; };
	bool OMXStop(bool lock = true);
	bool OMXStart(double pts, bool lock = true);
	bool OMXStep(int steps = 1, bool lock = true);
	bool OMXReset(bool lock = true);
	double OMXMediaTime(bool lock = true);
	double OMXClockAdjustment(bool lock = true);
	bool OMXMediaTime(double pts, bool lock = true);
	bool OMXPause(bool lock = true);
	bool OMXResume(bool lock = true);
	bool OMXSetSpeed(int speed, bool lock = true, bool pause_resume = false);
	int  OMXPlaySpeed() { return m_omx_speed; };
	COMXCoreComponent *GetOMXClock();
	bool OMXStateExecute(bool lock = true);
	void OMXStateIdle(bool lock = true);
	bool HDMIClockSync(bool lock = true);
	int64_t GetAbsoluteClock();
	double GetClock(bool interpolated = true);
	static void OMXSleep(unsigned int dwMilliSeconds);
};
