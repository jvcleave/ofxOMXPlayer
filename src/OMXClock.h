#pragma once

#include "Component.h"


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
		Component clockComponent;
	public:
		OMXClock();
		~OMXClock();
		void Lock();
		void UnLock();
		bool init(bool has_video, bool has_audio);
		void OMXDeinitialize();
		bool isPaused()
		{
			return m_pause;
		};
		bool stop(bool lock = true);
		bool start(double pts, bool lock = true);
		bool step(int steps = 1, bool lock = true);
		bool reset(bool lock = true);
		double getMediaTime(bool lock = true);
		double OMXClockAdjustment(bool lock = true);
		bool getMediaTime(double pts, bool lock = true);
		bool pause(bool lock = true);
		bool resume(bool lock = true);
		bool setSpeed(int speed, bool lock = true, bool pause_resume = false);
		int  getSpeed()
		{
			return m_omx_speed;
		};
		Component *getComponent();
		bool OMXStateExecute(bool lock = true);
		void setToIdleState(bool lock = true);
		bool doHDMIClockSync(bool lock = true);
		int64_t getAbsoluteClock();
		void sleep(unsigned int dwMilliSeconds);
};