#include "ofxOMXPlayerEngine.h"


ofxOMXPlayerEngine::ofxOMXPlayerEngine()
{
    
    videoWidth = 0;
    videoHeight = 0;
    hasVideo = false;
    hasAudio = false;
    packet = NULL;
    moviePath = "moviePath is undefined";
    
    bPlaying = false;
    duration = 0.0;
    nFrames = 0;
    
    useHDMIForAudio = true;
    loop_offset = 0;
    startpts = 0.0;
    
    didAudioOpen = false;
    didVideoOpen = false;
    isTextureEnabled = false;
    doLooping = false;
    videoPlayer = NULL;
    texturedPlayer = NULL;
    directPlayer = NULL;
    audioPlayer = NULL;
    clock = NULL;
    
    listener = NULL;
    
    loopCounter = 0;
    previousLoopOffset = 0;
    
    startFrame = 0;
    
    normalPlaySpeed = 1000;
    speedMultiplier = 1;
    doSeek = false;
    
    eglImage = NULL;
    
    
}

#pragma mark startup/setup

bool ofxOMXPlayerEngine::setup(ofxOMXPlayerSettings& settings)
{
    omxPlayerSettings = settings;
    moviePath = omxPlayerSettings.videoPath;
    useHDMIForAudio = omxPlayerSettings.useHDMIForAudio;
    doLooping = omxPlayerSettings.enableLooping;
    addListener(omxPlayerSettings.listener);
    
    //ofLogVerbose(__func__) << "moviePath is " << moviePath;
    isTextureEnabled = omxPlayerSettings.enableTexture;
    
    bool doSkipAvProbe = true;
    bool didOpenMovie = didReadFile(doSkipAvProbe);
    
    if(!didOpenMovie)
    {
        ofLogWarning(__func__) << "FAST PATH MOVE OPEN FAILED - LIKELY A STREAM, TRYING SLOW PATH";
        doSkipAvProbe = false;
        didOpenMovie = didReadFile(doSkipAvProbe);
    }
    
    if(didOpenMovie)
    {
        
        //ofLogVerbose(__func__) << "omxReader open moviePath PASS: " << moviePath;
        
        hasVideo = omxReader.getNumVideoStreams();
        int audioStreamCount = omxReader.getNumAudioStreams();
        
        if (audioStreamCount>0)
        {
            hasAudio = true;
            omxReader.getHints(OMXSTREAM_AUDIO, audioStreamInfo);
            //ofLogVerbose(__func__) << "HAS AUDIO";
        }
        else
        {
            //ofLogVerbose(__func__) << "NO AUDIO";
        }
        
        if (!omxPlayerSettings.enableAudio)
        {
            hasAudio = false;
        }
        
        if (hasVideo)
        {
            
            //again?
            omxReader.getHints(OMXSTREAM_VIDEO, videoStreamInfo);
            
            videoWidth = videoStreamInfo.width;
            videoHeight = videoStreamInfo.height;
            omxPlayerSettings.videoWidth = videoStreamInfo.width;
            omxPlayerSettings.videoHeight = videoStreamInfo.height;
            
            //ofLogVerbose(__func__) << "SET videoWidth: " << videoWidth;
            //ofLogVerbose(__func__) << "SET videoHeight: " << videoHeight;
            //ofLogVerbose(__func__) << "videoStreamInfo.nb_frames " <<videoStreamInfo.nb_frames;
            if(clock)
            {
                delete clock;
                clock = NULL;
            }
            clock = new OMXClock();
            
            if(clock->init(hasVideo, hasAudio))
            {
                //ofLogVerbose(__func__) << "clock Init PASS";
                return true;
            }
            else
            {
                ofLogError() << "clock Init FAIL";
                return false;
            }
        }
        else
        {
            ofLogError() << "Video streams detection FAIL";
            return false;
        }
    }
    else
    {
        ofLogError() << "omxReader open moviePath FAIL: " << moviePath;
        return false;
    }
}

bool ofxOMXPlayerEngine::didReadFile(bool doSkipAvProbe)
{
    bool passed = false;
    
    unsigned long long startTime = ofGetElapsedTimeMillis();
    
    bool didOpenMovie = omxReader.open(moviePath.c_str(), doSkipAvProbe);
    
    unsigned long long endTime = ofGetElapsedTimeMillis();
    ofLogNotice(__func__) << "didOpenMovie TOOK " << endTime-startTime << " MS";
    
    
    if(didOpenMovie)
    {
        omxReader.getHints(OMXSTREAM_VIDEO, videoStreamInfo);
        if(videoStreamInfo.width > 0 || videoStreamInfo.height > 0)
        {
            passed = true;
        }
    }
    return passed;
}


bool ofxOMXPlayerEngine::openPlayer(int startTimeInSeconds)
{
    
    if (isTextureEnabled)
    {
        if (!texturedPlayer)
        {
            texturedPlayer = new VideoPlayerTextured();
        }
        didVideoOpen = texturedPlayer->open(videoStreamInfo, clock, eglImage);
        videoPlayer = (BaseVideoPlayer*)texturedPlayer;
    }
    else
    {
        if (!directPlayer)
        {
            directPlayer = new VideoPlayerDirect();
        }
 
        didVideoOpen = directPlayer->open(videoStreamInfo, clock, omxPlayerSettings);
        videoPlayer = (BaseVideoPlayer*)directPlayer;
    }
    
    bPlaying = didVideoOpen;
    
    if (hasAudio)
    {
        string deviceString = "omx:hdmi";
        if (!useHDMIForAudio)
        {
            deviceString = "omx:local";
        }
        audioPlayer = new OMXAudioPlayer();
        didAudioOpen = audioPlayer->open(audioStreamInfo, clock, &omxReader, deviceString);
        if (didAudioOpen)
        {
            setVolume(omxPlayerSettings.initialVolume);
        }
        else
        {
            ofLogError(__func__) << " AUDIO PLAYER OPEN FAIL";
        }
    }
    
    
    if (isPlaying())
    {
        
        
        if(videoStreamInfo.nb_frames>0 && videoPlayer->getFPS()>0)
        {
            nFrames =videoStreamInfo.nb_frames;
            duration =videoStreamInfo.nb_frames / videoPlayer->getFPS();
            ofLogNotice(__func__) << "duration SET: " << duration;
        }
        else
        {
            
            //file is weird (like test.h264) and has no reported frames
        }
        if (startTimeInSeconds !=0 && omxReader.canSeek())
        {
            
            bool didSeek = omxReader.SeekTime(startTimeInSeconds * 1000.0f, 0, &startpts);
            if(didSeek)
            {
                startFrame = (int)videoPlayer->getFPS()*(int)startTimeInSeconds;
                ofLogNotice(__func__) << "Seeking start of video to " << startTimeInSeconds << " seconds, frame: " << startFrame;
            }else
            {
                ofLogError(__func__) << "COULD NOT SEEK TO " << startTimeInSeconds;
            }
        }
        
        clock->start(startpts);
        
        ofLogNotice(__func__) << "Opened video PASS";
        Create();
        return true;
    }
    else
    {
        ofLogError(__func__) << "Opened video FAIL";
        return false;
    }
}

#pragma mark threading
void ofxOMXPlayerEngine::process()
{
    while (!doStop)
    {
        
        if(!packet)
        {
            packet = omxReader.Read();
            if (packet && doLooping && packet->pts != DVD_NOPTS_VALUE)
            {
                packet->pts += loop_offset;
                packet->dts += loop_offset;
            }
        }
        
        bool isCacheEmpty = false;
        
        if (!packet)
        {
            if (hasAudio)
            {
                if (!audioPlayer->getCached() && !videoPlayer->getCached())
                {
                    isCacheEmpty = true;
                }
            }
            else
            {
                if (!videoPlayer->getCached())
                {
                    isCacheEmpty = true;
                }
            }
            
        }
        
        if (omxReader.getIsEOF() && !packet && isCacheEmpty)
        {
            videoPlayer->submitEOS();
            
        }
        
        if(doLooping && omxReader.getIsEOF() && !packet)
        {
            
            
            if (isCacheEmpty)
            {
                omxReader.SeekTime(0 * 1000.0f, AVSEEK_FLAG_BACKWARD, &startpts);
                
                packet = omxReader.Read();
                
                if(hasAudio)
                {
                    loop_offset = audioPlayer->getCurrentPTS();
                }
                else
                {
                    if(hasVideo)
                    {
                        loop_offset = videoPlayer->getCurrentPTS();
                    }
                }
                if (previousLoopOffset != loop_offset)
                {
                    
                    previousLoopOffset = loop_offset;
                    loopCounter++;                    
                    ofLog(OF_LOG_VERBOSE, "Loop offset : %8.02f\n", loop_offset / DVD_TIME_BASE);
                    
                    onVideoLoop();
                    
                }
                if (omxReader.wasFileRewound)
                {
                    omxReader.wasFileRewound = false;
                    onVideoLoop();
                }
                
            }
            else
            {
                clock->sleep(10);
                continue;
            }
            
        }
        else
        {
            if (!doLooping && omxReader.getIsEOF() && !packet && isCacheEmpty)
            {
                if (videoPlayer->EOS())
                {
                    onVideoEnd();
                    break;
                }
            }
            
        }
        
        
        if (doLooping && getCurrentFrame()>=getTotalNumFrames())
        {
            if (videoPlayer) 
            {
                videoPlayer->resetFrameCounter();
                startFrame = 0;
            }
        }
        if (hasAudio)
        {
            if(audioPlayer->getError())
            {
                ofLogError(__func__) << "audio player error.";
                hasAudio = false;
            }
        }
        
        
        if(hasVideo && packet && omxReader.isActive(OMXSTREAM_VIDEO, packet->stream_index))
        {
            if(videoPlayer->addPacket(packet))
            {
                packet = NULL;
            }
            else
            {
                clock->sleep(10);
            }
            
        }
        else if(hasAudio && packet && packet->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            if(audioPlayer->addPacket(packet))
            {
                packet = NULL;
            }
            else
            {
                clock->sleep(10);
            }
        }
        else
        {
            if(packet)
            {
                omxReader.freePacket(packet);
                packet = NULL;
            }else {
                clock->sleep(10);
            }
            
        }
    }
}


#pragma mark playback commands

void ofxOMXPlayerEngine::setNormalSpeed()
{
    lock();
    speedMultiplier = 1;
    clock->setSpeed(normalPlaySpeed);
    omxReader.setSpeed(normalPlaySpeed);
    unlock();
}

int ofxOMXPlayerEngine::increaseSpeed()
{
    
    lock();
    doSeek = true;
    
    if(speedMultiplier+1 <=4)
    {
        speedMultiplier++;
        int newSpeed = normalPlaySpeed*speedMultiplier;
        
        clock->setSpeed(newSpeed);
        omxReader.setSpeed(newSpeed);
    }
    unlock();
    return speedMultiplier;
}

void ofxOMXPlayerEngine::rewind()
{
    if(speedMultiplier-1 == 0)
    {
        speedMultiplier = -1;
    }
    else
    {
        speedMultiplier--;
    }
    
    
    if(speedMultiplier<-8)
    {
        speedMultiplier = 1;
    }
    int newSpeed = normalPlaySpeed*speedMultiplier;
    
    clock->setSpeed(newSpeed);
    omxReader.setSpeed(newSpeed);
    
}

void ofxOMXPlayerEngine::stepFrameForward()
{
    if (!isPaused())
    {
        setPaused(true);
    }
    clock->step(1);
}

void ofxOMXPlayerEngine::stop()
{
    setPaused(true);
}

bool ofxOMXPlayerEngine::setPaused(bool doPause)
{
    
    bool result = false;
    lock();
    if(doPause)
    {
        
        result =  clock->pause();
    }
    else
    {
        
        result =  clock->resume();
    }
    
    ofLogVerbose(__func__) << "result: " << result;
    unlock();
    return result;
}

void ofxOMXPlayerEngine::play()
{
    //ofLogVerbose(__func__) << "TODO: not sure what to do with this - reopen the player?";
}




#pragma mark getters

float ofxOMXPlayerEngine::getFPS()
{
    if (videoPlayer)
    {
        
        return videoPlayer->getFPS();
    }
    return 0;
}


float ofxOMXPlayerEngine::getDurationInSeconds()
{
    return duration;
}

int ofxOMXPlayerEngine::getCurrentFrame()
{
    if (videoPlayer) 
    {
        
        return videoPlayer->getCurrentFrame()+startFrame;
    }
    
    return 0;
}

int ofxOMXPlayerEngine::getTotalNumFrames()
{
    return nFrames;
}

int ofxOMXPlayerEngine::getWidth()
{
    return videoWidth;
}

int ofxOMXPlayerEngine::getHeight()
{
    return videoHeight;
}

double ofxOMXPlayerEngine::getMediaTime()
{
    double mediaTime = 0.0;
    if(isPlaying())
    {
        mediaTime = clock->getMediaTime();
    }
    
    return mediaTime;
}

bool ofxOMXPlayerEngine::isPaused()
{
    return clock->isPaused();
}


bool ofxOMXPlayerEngine::isPlaying()
{
    return bPlaying;
}

#pragma mark audio

void ofxOMXPlayerEngine::increaseVolume()
{
    if (!hasAudio || !didAudioOpen)
    {
        return;
    }
    
    float currentVolume = getVolume();
    currentVolume+=0.1;
    setVolume(currentVolume);
}


void ofxOMXPlayerEngine::decreaseVolume()
{
    if (!hasAudio || !didAudioOpen)
    {
        return;
    }
    
    float currentVolume = getVolume();
    currentVolume-=0.1;
    setVolume(currentVolume);
}

void ofxOMXPlayerEngine::setVolume(float volume)
{
    if (!hasAudio || !didAudioOpen)
    {
        return;
    }
    float value = ofMap(volume, 0.0, 1.0, -6000.0, 6000.0, true);
    audioPlayer->setCurrentVolume(value);
}

float ofxOMXPlayerEngine::getVolume()
{
    if (!hasAudio || !didAudioOpen || !audioPlayer)
    {
        return 0;
    }
    float value = ofMap(audioPlayer->getCurrentVolume(), -6000.0, 6000.0, 0.0, 1.0, true);
    return floorf(value * 100 + 0.5) / 100;
}

#pragma mark events

void ofxOMXPlayerEngine::addListener(ofxOMXPlayerListener* listener_)
{
    listener = listener_;
}

void ofxOMXPlayerEngine::removeListener()
{
    listener = NULL;
}



void ofxOMXPlayerEngine::onVideoLoop()
{
    
    
    if (listener != NULL)
    {
        
        ofxOMXPlayerListenerEventData eventData((void *)this);
        listener->onVideoLoop(eventData);
    }
}
void ofxOMXPlayerEngine::onVideoEnd()
{
    if (listener != NULL)
    {
        
        ofxOMXPlayerListenerEventData eventData((void *)this);
        listener->onVideoEnd(eventData);
    }
    
}

#pragma mark shutdown

ofxOMXPlayerEngine::~ofxOMXPlayerEngine()
{
    doStop = true;
    
    if(ThreadHandle())
    {
        StopThread("ofxOMXPlayerEngine");
    }
    
    bPlaying = false;
    
    
    if (listener)
    {
        listener = NULL;
    }
    
    if (texturedPlayer)
    {
        delete texturedPlayer;
        texturedPlayer = NULL;
    }
    if (directPlayer)
    {
        delete directPlayer;
        directPlayer = NULL;
    }
    
    videoPlayer = NULL;
    
    
    if (audioPlayer)
    {
        delete audioPlayer;
        audioPlayer = NULL;
    }
    
    if(packet)
    {
        omxReader.freePacket(packet);
        packet = NULL;
    }
    
    omxReader.close();
    
    delete clock;
    clock = NULL;
    
}


