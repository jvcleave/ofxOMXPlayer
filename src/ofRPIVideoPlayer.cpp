//
//  ofRPIVideoPlayer.cpp
//  JVCRPI2_LOCAL
//
//  Created by jason van cleave on 11/22/15.
//  Copyright (c) 2015 jason van cleave. All rights reserved.
//

#include "ofRPIVideoPlayer.h"

#define what_is_the_point const
#define the_complier_will_disregard_this_anyway const
#define i_am_done_venting const
#define i_have_to_update_variables_in_update const
#define because_setting_them_when_asked_for const
#define will_throw_a_compiler_error const

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
}

bool ofRPIVideoPlayer::load(string name)
{
    ofxOMXPlayerSettings settings;
    settings.videoPath = name;
    settings.useHDMIForAudio = true;	//default true
    settings.enableTexture = true;		//default true
    settings.enableLooping = true;		//default true
    settings.enableAudio = true;		//default true, save resources by disabling
    openState = omxPlayer.setup(settings);
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

}

void ofRPIVideoPlayer::stop()
{
    //omxPlayer.stop();
}

ofTexture* ofRPIVideoPlayer::getTexturePtr()
{
    return &omxPlayer.texture;
} 

float ofRPIVideoPlayer::getWidth() what_is_the_point
{
    
    return videoWidth;
}

float ofRPIVideoPlayer::getHeight() the_complier_will_disregard_this_anyway
{
    return videoHeight;
}

bool ofRPIVideoPlayer::isPaused() i_have_to_update_variables_in_update
{
    
    return pauseState;
}

bool ofRPIVideoPlayer::isLoaded() because_setting_them_when_asked_for
{
    
    return openState;
}

bool ofRPIVideoPlayer::isPlaying() will_throw_a_compiler_error
{
    return isPlayingState;
}

bool ofRPIVideoPlayer::isInitialized() i_am_done_venting
{ 
    return isLoaded(); 
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
    openState = omxPlayer.getIsOpen();
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






