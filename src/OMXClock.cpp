#include "OMXClock.h"

OMXClock::OMXClock()
{


	hasVideo   = false;
	hasAudio   = false;
	pauseState       = false;

	currentSpeed  = DVD_PLAYSPEED_NORMAL;

	pthread_mutex_init(&m_lock, NULL);

	//reset();
}

OMXClock::~OMXClock()
{
	deinit();

	pthread_mutex_destroy(&m_lock);
}

void OMXClock::lock()
{
	pthread_mutex_lock(&m_lock);
}

void OMXClock::unlock()
{
	pthread_mutex_unlock(&m_lock);
}

bool OMXClock::init(bool has_video, bool has_audio)
{
	OMX_ERRORTYPE error = OMX_ErrorNone;
	std::string componentName = "OMX.broadcom.clock";

	hasVideo = has_video;
	hasAudio = has_audio;

	pauseState       = false;

	if(!clockComponent.init(componentName, OMX_IndexParamOtherInit))
	{
		return false;
	}

	OMX_TIME_CONFIG_ACTIVEREFCLOCKTYPE refClock;
	OMX_INIT_STRUCTURE(refClock);

	if(hasAudio)
	{
		refClock.eClock = OMX_TIME_RefClockAudio;
	}
	else
	{
		refClock.eClock = OMX_TIME_RefClockVideo;
	}

	error = clockComponent.setConfig(OMX_IndexConfigTimeActiveRefClock, &refClock);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		return false;
	}

	return true;
}

void OMXClock::deinit()
{
	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return;
	}

	clockComponent.Deinitialize();
}

bool OMXClock::OMXStateExecute()
{
	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return false;
	}

	lock();

	if(clockComponent.getState() != OMX_StateExecuting)
	{
		OMX_ERRORTYPE error = OMX_ErrorNone;
		error = clockComponent.setState(OMX_StateExecuting);
        OMX_TRACE(error);
		if (error != OMX_ErrorNone)
		{
			unlock();
			return false;
		}
	}

	unlock();

	return true;
}

void OMXClock::setToIdleState()
{
	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return;
	}

	lock();
	OMX_ERRORTYPE error = OMX_ErrorNone;

	if(clockComponent.getState() == OMX_StateExecuting)
	{
		error = clockComponent.setState(OMX_StatePause);
        OMX_TRACE(error);

	}

	if(clockComponent.getState() != OMX_StateIdle)
	{
		error = clockComponent.setState(OMX_StateIdle);
        OMX_TRACE(error);

	}

	unlock();
}

Component* OMXClock::getComponent()
{
	if(!clockComponent.getHandle())
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return NULL;
	}

	return &clockComponent;
}

bool  OMXClock::stop()
{
	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return false;
	}


	lock();

	//ofLogVerbose(__func__) << "START";

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_TIME_CONFIG_CLOCKSTATETYPE clock;
	OMX_INIT_STRUCTURE(clock);

	clock.eState = OMX_TIME_ClockStateStopped;

	error = clockComponent.setConfig(OMX_IndexConfigTimeClockState, &clock);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		unlock();
		return false;
	}

	unlock();

	return true;
}

bool OMXClock::start(double pts)
{
	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return false;
	}

	lock();
    
	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_TIME_CONFIG_CLOCKSTATETYPE clock;
	OMX_INIT_STRUCTURE(clock);

	clock.eState = OMX_TIME_ClockStateRunning;
	clock.nStartTime = ToOMXTime((uint64_t)pts);

	error = clockComponent.setConfig(OMX_IndexConfigTimeClockState, &clock);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		unlock();
		return false;
	}

	unlock();

	return true;
}

bool OMXClock::step(int steps)
{
	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return false;
	}

	lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_PARAM_U32TYPE param;
	OMX_INIT_STRUCTURE(param);

	param.nPortIndex = OMX_ALL;
	param.nU32 = steps;

	error = clockComponent.setConfig(OMX_IndexConfigSingleStep, &param);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		unlock();
		return false;
	}

	unlock();

	return true;
}

bool OMXClock::reset()
{
	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return false;
	}

	lock();

	stop();
	start(0.0);

	unlock();

	return true;
}

double OMXClock::getMediaTime()
{
	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return 0;
	}

	lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	double pts = 0;

	OMX_TIME_CONFIG_TIMESTAMPTYPE timeStamp;
	OMX_INIT_STRUCTURE(timeStamp);
	timeStamp.nPortIndex = clockComponent.getInputPort();

	error = clockComponent.getConfig(OMX_IndexConfigTimeCurrentMediaTime, &timeStamp);
    OMX_TRACE(error);

	if(error != OMX_ErrorNone)
	{
		unlock();
		return 0;
	}

	pts = (double)FromOMXTime(timeStamp.nTimestamp);
	unlock();

	return pts;
}

// Set the media time, so calls to get media time use the updated value,
// useful after a seek so mediatime is updated immediately (rather than waiting for first decoded packet)
bool OMXClock::getMediaTime(double pts)
{
	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return false;
	}

	lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_INDEXTYPE index;
	OMX_TIME_CONFIG_TIMESTAMPTYPE timeStamp;
	OMX_INIT_STRUCTURE(timeStamp);
	timeStamp.nPortIndex = clockComponent.getInputPort();

	if(hasAudio)
	{
		index = OMX_IndexConfigTimeCurrentAudioReference;
	}
	else
	{
		index = OMX_IndexConfigTimeCurrentVideoReference;
	}

	timeStamp.nTimestamp = ToOMXTime(pts);

	error = clockComponent.setConfig(index, &timeStamp);
    OMX_TRACE(error);

	if(error != OMX_ErrorNone)
	{
		unlock();
		return false;
	}
	unlock();

	return true;
}

bool OMXClock::pause()
{
	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return false;
	}

	if(!pauseState)
	{
		lock();

		if (setSpeed(0, true))
		{
			pauseState = true;
		}

		unlock();
	}
	return pauseState == true;
}

bool OMXClock::resume()
{
	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return false;
	}

	if(pauseState)
	{
		lock();

		if (setSpeed(currentSpeed, true))
		{
			pauseState = false;
		}

		unlock();
	}
	return pauseState == false;
}

#define TRICKPLAY(speed) (speed < 0 || speed > 1.2 * DVD_PLAYSPEED_NORMAL)

bool OMXClock::setSpeed(int speed, bool doResume /* = false */)
{
	ofLog(OF_LOG_VERBOSE, "OMXClock::setSpeed(%d)", speed);

	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return false;
	}

	lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_TIME_CONFIG_SCALETYPE scaleType;
	OMX_INIT_STRUCTURE(scaleType);


	OMX_TIME_CONFIG_ACTIVEREFCLOCKTYPE refClock;
	OMX_INIT_STRUCTURE(refClock);

	if(hasAudio && !TRICKPLAY(speed))
	{
		refClock.eClock = OMX_TIME_RefClockAudio;
	}
	else
	{
		refClock.eClock = OMX_TIME_RefClockVideo;
	}

	error = clockComponent.setConfig(OMX_IndexConfigTimeActiveRefClock, &refClock);
    OMX_TRACE(error);

	if(error != OMX_ErrorNone)
	{
		return false;
	}
	if (TRICKPLAY(speed))
	{
		step(-1);
	}
	else
	{
		step(0);
	}

	if (0 && TRICKPLAY(speed))
	{
		scaleType.xScale = 0;
	}
	else
	{
		scaleType.xScale = (speed << 16) / DVD_PLAYSPEED_NORMAL;
	}
	error = clockComponent.setConfig(OMX_IndexConfigTimeScale, &scaleType);
    OMX_TRACE(error);

	if(error != OMX_ErrorNone)
	{
		unlock();
		return false;
	}

	if (!doResume)
	{
		currentSpeed = speed;
	}

	unlock();

	return true;
}

bool OMXClock::doHDMIClockSync()
{
	if(clockComponent.getHandle() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return false;
	}

	lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_CONFIG_LATENCYTARGETTYPE latencyTarget;
	OMX_INIT_STRUCTURE(latencyTarget);

	latencyTarget.nPortIndex = OMX_ALL;
	latencyTarget.bEnabled = OMX_TRUE;
	latencyTarget.nFilter = 10;
	latencyTarget.nTarget = 0;
	latencyTarget.nShift = 3;
	latencyTarget.nSpeedFactor = -200;
	latencyTarget.nInterFactor = 100;
	latencyTarget.nAdjCap = 100;

	error = clockComponent.setConfig(OMX_IndexConfigLatencyTarget, &latencyTarget);
    OMX_TRACE(error);

	if(error != OMX_ErrorNone)
	{
        unlock();
		return false;
	}

	unlock();

	return true;
}

void  OMXClock::sleep(unsigned int dwMilliSeconds)
{
	struct timespec req;
	req.tv_sec = dwMilliSeconds / 1000;
	req.tv_nsec = (dwMilliSeconds % 1000) * 1000000;

	while ( nanosleep(&req, &req) == -1 && errno == EINTR && (req.tv_nsec > 0 || req.tv_sec > 0));
}

static int64_t CurrentHostCounter(void)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return( ((int64_t)now.tv_sec * 1000000000L) + now.tv_nsec );
}

int64_t OMXClock::getAbsoluteClock()
{
	return CurrentHostCounter();
}
