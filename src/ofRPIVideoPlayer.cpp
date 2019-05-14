//
//  ofRPIVideoPlayer.cpp
//  JVCRPI2_LOCAL
//
//  Created by jason van cleave on 11/22/15.
//  Copyright (c) 2015 jason van cleave. All rights reserved.
//

#include "ofRPIVideoPlayer.h"


void ofRPIVideoPlayer::onVideoEnd(ofxOMXPlayer* player)
{
    videoHasEnded = true;
}

void ofRPIVideoPlayer::onVideoLoop(ofxOMXPlayer* player)
{
    //videoHasEnded = true;

}

ofRPIVideoPlayer::ofRPIVideoPlayer()
{
    pixelFormat = OF_PIXELS_RGBA;
    videoWidth = 0;
    videoHeight=0;
    pauseState = false;
    isPlayingState = false;
    hasNewFrame = true;
    openState = false;
    doPixels = false;
    videoHasEnded = false;
}

bool ofRPIVideoPlayer::load(string name)
{
    
    settings.videoPath = name;
    settings.useHDMIForAudio = true;	//default true
    settings.enableTexture = true;		//default true
    settings.enableLooping = true;		//default true
    settings.enableAudio = true;		//default true, save resources by disabling
    
    
    settings.listener = this;
    bool result = openOMXPlayer(settings);
    return result;
    
    
}

bool ofRPIVideoPlayer::loadWithSettings(ofxOMXPlayerSettings newSettings)
{
    newSettings.listener = this;
    return openOMXPlayer(newSettings);
}

bool ofRPIVideoPlayer::openOMXPlayer(ofxOMXPlayerSettings settings_)
{
    settings = settings_;
    openState = omxPlayer.setup(settings);
    videoHasEnded = false;
    update();
    return openState;
}

void ofRPIVideoPlayer::enablePixels()
{
    doPixels = true;
}

void ofRPIVideoPlayer::disablePixels()
{
    doPixels = false;
}

void ofRPIVideoPlayer::loadAsync(string name)
{
    load(name);
}

void ofRPIVideoPlayer::play()
{
    videoHasEnded = false;
}

void ofRPIVideoPlayer::stop()
{
    //omxPlayer.stop();
}


void ofRPIVideoPlayer::setVolume(float volume)
{
    omxPlayer.setVolumeNormalized(volume);
}

float ofRPIVideoPlayer::getVolume()
{
    return omxPlayer.getVolumeNormalized();
}


void ofRPIVideoPlayer::setPaused(bool doPause)
{
    omxPlayer.setPaused(doPause);
}

int ofRPIVideoPlayer::getCurrentFrame() 
{
    return omxPlayer.getCurrentFrame();
}

int ofRPIVideoPlayer::getTotalNumFrames()
{
    return omxPlayer.getTotalNumFrames();
}

ofTexture* ofRPIVideoPlayer::getTexturePtr()
{
    return &omxPlayer.getTextureReference();
} 

float ofRPIVideoPlayer::getWidth() const
{
    
    return videoWidth;
}

float ofRPIVideoPlayer::getHeight() const
{
    return videoHeight;
}

bool ofRPIVideoPlayer::isPaused() const
{
    
    return pauseState;
}

bool ofRPIVideoPlayer::isLoaded() const
{
    
    return openState;
}

bool ofRPIVideoPlayer::isPlaying() const
{
    return isPlayingState;
}

bool ofRPIVideoPlayer::isInitialized() const
{ 
    return isLoaded(); 
}

bool ofRPIVideoPlayer::getIsMovieDone() const
{
    return videoHasEnded;
}


ofPixels& ofRPIVideoPlayer::getPixels()
{
    if(!pixels.isAllocated())
    {
        pixels.allocate(getWidth(), getHeight(), getPixelFormat());
        pixels.setFromExternalPixels(omxPlayer.getPixels(), getWidth(), getHeight(), 4);
    }
    return pixels;
}

const ofPixels& ofRPIVideoPlayer::getPixels() const
{
    return pixels;

}


void ofRPIVideoPlayer::update()
{
    videoWidth = omxPlayer.getWidth();
    videoHeight = omxPlayer.getHeight();
    pauseState = omxPlayer.isPaused();
    openState = omxPlayer.isOpen();
    isPlayingState = !pauseState;
    hasNewFrame = omxPlayer.isFrameNew();
    if (doPixels && hasNewFrame) 
    {
        omxPlayer.updatePixels();
    }
    
}

void ofRPIVideoPlayer::draw(float x, float y)
{
    omxPlayer.draw(x, y, getWidth(), getHeight());
}


void ofRPIVideoPlayer::draw(float x, float y, float w, float h)
{
    omxPlayer.draw(x, y, w, h);
}


bool ofRPIVideoPlayer::isFrameNew() const
{
    return hasNewFrame;
}

void ofRPIVideoPlayer::close()
{
    omxPlayer.close();
}

bool ofRPIVideoPlayer::setPixelFormat(ofPixelFormat pixelFormat)
{
    return false;
}

ofPixelFormat ofRPIVideoPlayer::getPixelFormat() const
{
    return pixelFormat;
}

void ofRPIVideoPlayer::setLoopState(ofLoopType requestedState)
{
    switch (requestedState) 
    {
        case OF_LOOP_NORMAL:
        {
            if(!omxPlayer.isLoopingEnabled()) 
            {
                omxPlayer.enableLooping();
            }
            break;
        }
        case OF_LOOP_NONE:
        {
            if(omxPlayer.isLoopingEnabled()) 
            {
                omxPlayer.disableLooping();
            }
            break;
        }  
        case OF_LOOP_PALINDROME:
        {
            ofLogWarning(__func__) << "OF_LOOP_PALINDROME is not supported";
            break;
        }     
        default:
        {
            ofLogError(__func__) << "UNKNOWN ofLoopType: " << requestedState;
            break;
        }
    }
    
}
