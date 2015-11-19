#include "OMXClock.h"

OMXClock::OMXClock()
{


	m_has_video   = false;
	m_has_audio   = false;
	m_pause       = false;

	m_omx_speed  = DVD_PLAYSPEED_NORMAL;

	pthread_mutex_init(&m_lock, NULL);

	OMXReset();
}

OMXClock::~OMXClock()
{
	OMXDeinitialize();

	pthread_mutex_destroy(&m_lock);
}

void OMXClock::Lock()
{
	pthread_mutex_lock(&m_lock);
}

void OMXClock::UnLock()
{
	pthread_mutex_unlock(&m_lock);
}

bool OMXClock::OMXInitialize(bool has_video, bool has_audio)
{
	OMX_ERRORTYPE error = OMX_ErrorNone;
	std::string componentName = "OMX.broadcom.clock";

	m_has_video = has_video;
	m_has_audio = has_audio;

	m_pause       = false;

	if(!m_omx_clock.init(componentName, OMX_IndexParamOtherInit))
	{
		return false;
	}

	OMX_TIME_CONFIG_ACTIVEREFCLOCKTYPE refClock;
	OMX_INIT_STRUCTURE(refClock);

	if(m_has_audio)
	{
		refClock.eClock = OMX_TIME_RefClockAudio;
	}
	else
	{
		refClock.eClock = OMX_TIME_RefClockVideo;
	}

	error = m_omx_clock.setConfig(OMX_IndexConfigTimeActiveRefClock, &refClock);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << " setting OMX_IndexConfigTimeCurrentAudioReference";
		return false;
	}

	return true;
}

void OMXClock::OMXDeinitialize()
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		return;
	}

	m_omx_clock.Deinitialize(true);
}

bool OMXClock::OMXStateExecute(bool lock /* = true */)
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		return false;
	}

	if(lock)
	{
		Lock();
	}

	if(m_omx_clock.getState() != OMX_StateExecuting)
	{
		OMX_ERRORTYPE error = OMX_ErrorNone;
		error = m_omx_clock.setState(OMX_StateExecuting);
        OMX_TRACE(error);
		if (error != OMX_ErrorNone)
		{
			if(lock)
			{
				UnLock();
			}
			return false;
		}
	}

	if(lock)
	{
		UnLock();
	}

	return true;
}

void OMXClock::OMXStateIdle(bool lock /* = true */)
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		ofLogError(__func__) << "NO CLOCK YET";
		return;
	}

	if(lock)
	{
		Lock();
	}
	OMX_ERRORTYPE error = OMX_ErrorNone;

	if(m_omx_clock.getState() == OMX_StateExecuting)
	{
		error = m_omx_clock.setState(OMX_StatePause);
        OMX_TRACE(error);

	}

	if(m_omx_clock.getState() != OMX_StateIdle)
	{
		error = m_omx_clock.setState(OMX_StateIdle);
        OMX_TRACE(error);

	}

	if(lock)
	{
		UnLock();
	}
}

Component *OMXClock::GetOMXClock()
{
	if(!m_omx_clock.GetComponent())
	{
		return NULL;
	}

	return &m_omx_clock;
}

bool  OMXClock::OMXStop(bool lock /* = true */)
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		return false;
	}


	if(lock)
	{
		Lock();
	}

	//ofLogVerbose(__func__) << "START";

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_TIME_CONFIG_CLOCKSTATETYPE clock;
	OMX_INIT_STRUCTURE(clock);

	clock.eState = OMX_TIME_ClockStateStopped;

	error = m_omx_clock.setConfig(OMX_IndexConfigTimeClockState, &clock);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		if(lock)
		{
			UnLock();
		}
		return false;
	}

	if(lock)
	{
		UnLock();
	}

	return true;
}

bool OMXClock::OMXStart(double pts, bool lock /* = true */)
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		return false;
	}

	if(lock)
	{
		Lock();
	}
    
	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_TIME_CONFIG_CLOCKSTATETYPE clock;
	OMX_INIT_STRUCTURE(clock);

	clock.eState = OMX_TIME_ClockStateRunning;
	clock.nStartTime = ToOMXTime((uint64_t)pts);

	error = m_omx_clock.setConfig(OMX_IndexConfigTimeClockState, &clock);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		if(lock)
		{
			UnLock();
		}
		return false;
	}

	if(lock)
	{
		UnLock();
	}

	return true;
}

bool OMXClock::OMXStep(int steps /* = 1 */, bool lock /* = true */)
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		return false;
	}

	if(lock)
	{
		Lock();
	}

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_PARAM_U32TYPE param;
	OMX_INIT_STRUCTURE(param);

	param.nPortIndex = OMX_ALL;
	param.nU32 = steps;

	error = m_omx_clock.setConfig(OMX_IndexConfigSingleStep, &param);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		if(lock)
		{
			UnLock();
		}
		return false;
	}

	if(lock)
	{
		UnLock();
	}

	return true;
}

bool OMXClock::OMXReset(bool lock /* = true */)
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		return false;
	}

	if(lock)
	{
		Lock();
	}

	OMXStop(false);
	OMXStart(0.0, false);

	if(lock)
	{
		UnLock();
	}

	return true;
}

double OMXClock::OMXMediaTime(bool lock /* = true */)
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		return 0;
	}

	if(lock)
	{
		Lock();
	}

	OMX_ERRORTYPE error = OMX_ErrorNone;
	double pts = 0;

	OMX_TIME_CONFIG_TIMESTAMPTYPE timeStamp;
	OMX_INIT_STRUCTURE(timeStamp);
	timeStamp.nPortIndex = m_omx_clock.GetInputPort();

	error = m_omx_clock.getConfig(OMX_IndexConfigTimeCurrentMediaTime, &timeStamp);
    OMX_TRACE(error);

	if(error != OMX_ErrorNone)
	{
		if(lock)
		{
			UnLock();
		}
		return 0;
	}

	pts = (double)FromOMXTime(timeStamp.nTimestamp);
	if(lock)
	{
		UnLock();
	}

	return pts;
}

double OMXClock::OMXClockAdjustment(bool lock /* = true */)
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		return 0;
	}

	if(lock)
	{
		Lock();
	}

	OMX_ERRORTYPE error = OMX_ErrorNone;
	double pts = 0;

	OMX_TIME_CONFIG_TIMESTAMPTYPE timeStamp;
	OMX_INIT_STRUCTURE(timeStamp);
	timeStamp.nPortIndex = m_omx_clock.GetInputPort();

	error = m_omx_clock.getConfig(OMX_IndexConfigClockAdjustment, &timeStamp);
    OMX_TRACE(error);

	if(error != OMX_ErrorNone)
	{
		if(lock)
		{
			UnLock();
		}
		return 0;
	}

	pts = (double)FromOMXTime(timeStamp.nTimestamp);
	
	if(lock)
	{
		UnLock();
	}

	return pts;
}


// Set the media time, so calls to get media time use the updated value,
// useful after a seek so mediatime is updated immediately (rather than waiting for first decoded packet)
bool OMXClock::OMXMediaTime(double pts, bool lock /* = true*/)
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		return false;
	}

	if(lock)
	{
		Lock();
	}

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_INDEXTYPE index;
	OMX_TIME_CONFIG_TIMESTAMPTYPE timeStamp;
	OMX_INIT_STRUCTURE(timeStamp);
	timeStamp.nPortIndex = m_omx_clock.GetInputPort();

	if(m_has_audio)
	{
		index = OMX_IndexConfigTimeCurrentAudioReference;
	}
	else
	{
		index = OMX_IndexConfigTimeCurrentVideoReference;
	}

	timeStamp.nTimestamp = ToOMXTime(pts);

	error = m_omx_clock.setConfig(index, &timeStamp);
    OMX_TRACE(error);

	if(error != OMX_ErrorNone)
	{
		if(lock)
		{
			UnLock();
		}
		return false;
	}
	if(lock)
	{
		UnLock();
	}

	return true;
}

bool OMXClock::OMXPause(bool lock /* = true */)
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		return false;
	}

	if(!m_pause)
	{
		if(lock)
		{
			Lock();
		}

		if (OMXSetSpeed(0, false, true))
		{
			m_pause = true;
		}

		if(lock)
		{
			UnLock();
		}
	}
	return m_pause == true;
}

bool OMXClock::OMXResume(bool lock /* = true */)
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		return false;
	}

	if(m_pause)
	{
		if(lock)
		{
			Lock();
		}

		if (OMXSetSpeed(m_omx_speed, false, true))
		{
			m_pause = false;
		}

		if(lock)
		{
			UnLock();
		}
	}
	return m_pause == false;
}

#define TRICKPLAY(speed) (speed < 0 || speed > 1.2 * DVD_PLAYSPEED_NORMAL)

bool OMXClock::OMXSetSpeed(int speed, bool lock /* = true */, bool pause_resume /* = false */)
{
	ofLog(OF_LOG_VERBOSE, "OMXClock::OMXSetSpeed(%d)", speed);

	if(m_omx_clock.GetComponent() == NULL)
	{
		return false;
	}

	if(lock)
	{
		Lock();
	}

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_TIME_CONFIG_SCALETYPE scaleType;
	OMX_INIT_STRUCTURE(scaleType);


	OMX_TIME_CONFIG_ACTIVEREFCLOCKTYPE refClock;
	OMX_INIT_STRUCTURE(refClock);

	if(m_has_audio && !TRICKPLAY(speed))
	{
		refClock.eClock = OMX_TIME_RefClockAudio;
	}
	else
	{
		refClock.eClock = OMX_TIME_RefClockVideo;
	}

	error = m_omx_clock.setConfig(OMX_IndexConfigTimeActiveRefClock, &refClock);
    OMX_TRACE(error);

	if(error != OMX_ErrorNone)
	{
		return false;
	}
	if (TRICKPLAY(speed))
	{
		OMXStep(-1, false);
	}
	else
	{
		OMXStep(0, false);
	}

	if (0 && TRICKPLAY(speed))
	{
		scaleType.xScale = 0;
	}
	else
	{
		scaleType.xScale = (speed << 16) / DVD_PLAYSPEED_NORMAL;
	}
	error = m_omx_clock.setConfig(OMX_IndexConfigTimeScale, &scaleType);
    OMX_TRACE(error);

	if(error != OMX_ErrorNone)
	{
		if(lock)
		{
			UnLock();
		}
		return false;
	}

	if (!pause_resume)
	{
		m_omx_speed = speed;
	}

	if(lock)
	{
		UnLock();
	}

	return true;
}

bool OMXClock::HDMIClockSync(bool lock /* = true */)
{
	if(m_omx_clock.GetComponent() == NULL)
	{
		return false;
	}

	if(lock)
	{
		Lock();
	}

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

	error = m_omx_clock.setConfig(OMX_IndexConfigLatencyTarget, &latencyTarget);
    OMX_TRACE(error);

	if(error != OMX_ErrorNone)
	{
        if(lock)
		{
			UnLock();
		}
		return false;
	}

	if(lock)
	{
		UnLock();
	}

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

int64_t OMXClock::GetAbsoluteClock()
{
	return CurrentHostCounter();
}

double OMXClock::GetClock(bool interpolated /*= true*/)
{
	return CurrentHostCounter();
	//return OMXMediaTime();
}