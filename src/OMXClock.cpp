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
	const std::string componentName = "OMX.broadcom.clock";

	m_has_video = has_video;
	m_has_audio = has_audio;

	m_pause       = false;

	if(!m_omx_clock.Initialize(componentName, OMX_IndexParamOtherInit))
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

	error = m_omx_clock.SetConfig(OMX_IndexConfigTimeActiveRefClock, &refClock);
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

	if(m_omx_clock.GetState() != OMX_StateExecuting)
	{
		OMX_ERRORTYPE error = OMX_ErrorNone;
		error = m_omx_clock.SetStateForComponent(OMX_StateExecuting);
		if (error != OMX_ErrorNone)
		{
			ofLogError(__func__) << "m_omx_clock GET OMX_StateExecuting FAIL error:" << COMXCore::getOMXError(error);
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
	//ofLogVerbose(__func__) << "OMXClock::OMXStateIdle START";
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

	if(m_omx_clock.GetState() == OMX_StateExecuting)
	{
		error = m_omx_clock.SetStateForComponent(OMX_StatePause);
		if (error != OMX_ErrorNone)
		{
			ofLogError(__func__) << "SetStateForComponent OMX_StatePause FAIL: " << COMXCore::getOMXError(error);
		}
		else
		{
			//ofLogVerbose(__func__) << "SetStateForComponent OMX_StatePause PASS";
		}
	}

	if(m_omx_clock.GetState() != OMX_StateIdle)
	{
		error = m_omx_clock.SetStateForComponent(OMX_StateIdle);
		if (error != OMX_ErrorNone)
		{
			ofLogError(__func__) << "SetStateForComponent OMX_StateIdle FAIL: " << COMXCore::getOMXError(error);
		}
		else
		{
			//ofLogVerbose(__func__) << "m_omx_clock SetStateForComponent OMX_StateIdle PASS";
		}
	}

	if(lock)
	{
		UnLock();
	}
	//ofLogVerbose(__func__) << "OMXClock::OMXStateIdle END";
}

COMXCoreComponent *OMXClock::GetOMXClock()
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
		//ofLogVerbose(__func__) << "NO CLOCK - RETURNING EARLY";
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

	error = m_omx_clock.SetConfig(OMX_IndexConfigTimeClockState, &clock);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "SetConfig OMX_IndexConfigTimeClockState FAIL: " << COMXCore::getOMXError(error);
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

	//ofLogVerbose(__func__) << "END";
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

	ofLogNotice(__func__) << "at pts: " << pts;

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_TIME_CONFIG_CLOCKSTATETYPE clock;
	OMX_INIT_STRUCTURE(clock);

	clock.eState = OMX_TIME_ClockStateRunning;
	clock.nStartTime = ToOMXTime((uint64_t)pts);

	error = m_omx_clock.SetConfig(OMX_IndexConfigTimeClockState, &clock);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "SetConfig OMX_IndexConfigTimeClockState FAIL: " << COMXCore::getOMXError(error);
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

	error = m_omx_clock.SetConfig(OMX_IndexConfigSingleStep, &param);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "SetConfig OMX_IndexConfigSingleStep FAIL: " << COMXCore::getOMXError(error);
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

	error = m_omx_clock.GetConfig(OMX_IndexConfigTimeCurrentMediaTime, &timeStamp);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "GetConfig OMX_IndexConfigTimeCurrentMediaTime FAIL: " << COMXCore::getOMXError(error);
		if(lock)
		{
			UnLock();
		}
		return 0;
	}

	pts = (double)FromOMXTime(timeStamp.nTimestamp);
	//ofLogVerbose(__func__) << " nTimestamp: " << (double)FromOMXTime(timeStamp.nTimestamp) << " pts: " << pts;
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

	error = m_omx_clock.GetConfig(OMX_IndexConfigClockAdjustment, &timeStamp);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "GetConfig OMX_IndexConfigClockAdjustment FAIL: " << COMXCore::getOMXError(error);
		if(lock)
		{
			UnLock();
		}
		return 0;
	}

	pts = (double)FromOMXTime(timeStamp.nTimestamp);
	
	//ofLogVerbose(__func__) << "nTimestamp: " << (double)FromOMXTime(timeStamp.nTimestamp) << " pts " << pts;
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

	error = m_omx_clock.SetConfig(index, &timeStamp);
	if(error != OMX_ErrorNone)
	{
		ofLog(OF_LOG_ERROR, "OMXClock::OMXMediaTime error setting %s", index == OMX_IndexConfigTimeCurrentAudioReference ?
		      "OMX_IndexConfigTimeCurrentAudioReference":"OMX_IndexConfigTimeCurrentVideoReference");
		ofLogError(__func__) << COMXCore::getOMXError(error);
		if(lock)
		{
			UnLock();
		}
		return false;
	}

	ofLog(OF_LOG_VERBOSE, "OMXClock::OMXMediaTime set config %s = %.2f (%.2f)", index == OMX_IndexConfigTimeCurrentAudioReference ?
	      "OMX_IndexConfigTimeCurrentAudioReference":"OMX_IndexConfigTimeCurrentVideoReference", pts, OMXMediaTime(false));

	if(lock)
	{
		UnLock();
	}

	return true;
}

bool OMXClock::OMXPause(bool lock /* = true */)
{
	//ofLogVerbose(__func__) << "START";
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
	//ofLogVerbose(__func__) << "END";
	return m_pause == true;
}

bool OMXClock::OMXResume(bool lock /* = true */)
{
	//ofLogVerbose(__func__) << "START";
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
	//ofLogVerbose(__func__) << "END";
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

	error = m_omx_clock.SetConfig(OMX_IndexConfigTimeActiveRefClock, &refClock);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "SetConfig OMX_IndexConfigTimeActiveRefClock FAIL: " << COMXCore::getOMXError(error);
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
	error = m_omx_clock.SetConfig(OMX_IndexConfigTimeScale, &scaleType);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "SetConfig OMX_IndexConfigTimeScale FAIL: " << COMXCore::getOMXError(error);
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

	error = m_omx_clock.SetConfig(OMX_IndexConfigLatencyTarget, &latencyTarget);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "SetConfig OMX_IndexConfigLatencyTarget FAIL: " << COMXCore::getOMXError(error);
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