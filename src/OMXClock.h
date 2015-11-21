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
		bool              pauseState;
		bool              hasVideo;
		bool              hasAudio;
		int               currentSpeed;
		pthread_mutex_t   m_lock;
	private:
		Component clockComponent;
        void lock();
        void unlock();
	public:
		OMXClock();
		~OMXClock();

		bool init(bool has_video, bool has_audio);

        bool isPaused()
		{
			return pauseState;
		};
		bool stop();
		bool start(double pts);
		bool step(int steps = 1);
		bool reset();
		double getMediaTime();
		bool setMediaTime(double pts);
		bool pause();
		bool resume();
		bool setSpeed(int speed, bool doResume = false);
		int  getSpeed()
		{
			return currentSpeed;
		};
		Component* getComponent();
		bool enableHDMISync();
		int64_t getAbsoluteClock();
		void sleep(unsigned int dwMilliSeconds);
};