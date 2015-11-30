#include "OMXClock.h"

OMXClock::OMXClock()
{


    hasVideo   = false;
    hasAudio   = false;
    pauseState = false;

    currentSpeed  = DVD_PLAYSPEED_NORMAL;
    previousSpeed = currentSpeed;
    pthread_mutex_init(&m_lock, NULL);

    //reset();
}

OMXClock::~OMXClock()
{
    clockComponent.Deinitialize(__func__);
    pthread_mutex_destroy(&m_lock);
}

void OMXClock::lock()
{
    //ofLogVerbose(__func__) << "";
    pthread_mutex_lock(&m_lock);
}

void OMXClock::unlock()
{
    //ofLogVerbose(__func__) << "";
    pthread_mutex_unlock(&m_lock);
}

void OMXClock::lock(string caller)
{
    ofLogVerbose(__func__) << " " << caller;
    lock();
}

void OMXClock::unlock(string caller)
{
    ofLogVerbose(__func__) << " " << caller;
    unlock();
}


bool OMXClock::init(bool has_video, bool has_audio)
{
    OMX_ERRORTYPE error = OMX_ErrorNone;
    std::string componentName = "OMX.broadcom.clock";

    hasVideo = has_video;
    hasAudio = has_audio;

    pauseState = false;

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


    lock(__func__);

    //ofLogVerbose(__func__) << "START";

    OMX_ERRORTYPE error = OMX_ErrorNone;
    OMX_TIME_CONFIG_CLOCKSTATETYPE clock;
    OMX_INIT_STRUCTURE(clock);

    clock.eState = OMX_TIME_ClockStateStopped;

    error = clockComponent.setConfig(OMX_IndexConfigTimeClockState, &clock);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
        unlock(__func__);
        return false;
    }

    unlock(__func__);

    return true;
}

bool OMXClock::start(double pts)
{
    if(clockComponent.getHandle() == NULL)
    {
        ofLogError(__func__) << "NO CLOCK YET";
        return false;
    }

    lock(__func__);
    
    OMX_ERRORTYPE error = clockComponent.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if (error != OMX_ErrorNone)
    {
        unlock(__func__);
        return false;
    }
    OMX_TIME_CONFIG_CLOCKSTATETYPE clock;
    OMX_INIT_STRUCTURE(clock);

    clock.eState = OMX_TIME_ClockStateRunning;
    clock.nStartTime = ToOMXTime((uint64_t)pts);

    error = clockComponent.setConfig(OMX_IndexConfigTimeClockState, &clock);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
        unlock(__func__);
        return false;
    }

    unlock(__func__);

    return true;
}

bool OMXClock::step(int steps)
{
    if(clockComponent.getHandle() == NULL)
    {
        ofLogError(__func__) << "NO CLOCK YET";
        return false;
    }

    //lock(__func__);

    OMX_ERRORTYPE error = OMX_ErrorNone;
    OMX_PARAM_U32TYPE param;
    OMX_INIT_STRUCTURE(param);

    param.nPortIndex = OMX_ALL;
    param.nU32 = steps;

    error = clockComponent.setConfig(OMX_IndexConfigSingleStep, &param);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
        unlock(__func__);
        return false;
    }

    //unlock(__func__);

    return true;
}

bool OMXClock::reset()
{
    if(clockComponent.getHandle() == NULL)
    {
        ofLogError(__func__) << "NO CLOCK YET";
        return false;
    }

    lock(__func__);

    stop();
    start(0.0);

    unlock(__func__);

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
        unlock(__func__);
        return 0;
    }

    pts = (double)FromOMXTime(timeStamp.nTimestamp);
    unlock();

    return pts;
}

// Set the media time, so calls to get media time use the updated value,
// useful after a seek so mediatime is updated immediately (rather than waiting for first decoded packet)
bool OMXClock::setMediaTime(double pts)
{
    if(clockComponent.getHandle() == NULL)
    {
        ofLogError(__func__) << "NO CLOCK YET";
        return false;
    }

    lock(__func__);

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
        unlock(__func__);
        return false;
    }
    unlock(__func__);

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
        lock(__func__);
        ofLogVerbose(__func__) << "currentSpeed: " << currentSpeed;
        previousSpeed = currentSpeed;
        if (setSpeed(0, false))
        {
            pauseState = true;
        }

        unlock(__func__);
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
        lock(__func__);

        ofLogVerbose(__func__) << "currentSpeed: " << currentSpeed;
        ofLogVerbose(__func__) << "previousSpeed: " << previousSpeed;
        if (setSpeed(previousSpeed, true))
        {
            pauseState = false;
        }

        unlock(__func__);
    }
    return pauseState == false;
}


bool OMXClock::setSpeed(int speed, bool doResume /* = false */)
{
    ofLogVerbose(__func__) << "speed: " << speed << " doResume: " << doResume;
    if(clockComponent.getHandle() == NULL)
    {
        ofLogError(__func__) << "NO CLOCK YET";
        return false;
    }

    //lock(__func__);

    OMX_ERRORTYPE error = OMX_ErrorNone;
    OMX_TIME_CONFIG_SCALETYPE scaleType;
    OMX_INIT_STRUCTURE(scaleType);


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

    step(0);
    scaleType.xScale = (speed << 16) / DVD_PLAYSPEED_NORMAL;  

    error = clockComponent.setConfig(OMX_IndexConfigTimeScale, &scaleType);
    OMX_TRACE(error);

    if(error != OMX_ErrorNone)
    {
        //unlock(__func__);
        return false;
    }

    currentSpeed = speed;

    //unlock(__func__);

    return true;
}

bool OMXClock::enableHDMISync()
{
    if(clockComponent.getHandle() == NULL)
    {
        ofLogError(__func__) << "NO CLOCK YET";
        return false;
    }

    lock(__func__);

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
        unlock(__func__);
        return false;
    }

    unlock(__func__);

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
