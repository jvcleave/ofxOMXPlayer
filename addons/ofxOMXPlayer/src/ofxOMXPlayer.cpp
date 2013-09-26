/*
 *  ofxOMXPlayer.cpp
 *  openFrameworksRPi
 *
 *  Created by jason van cleave on 9/8/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */

#include "ofxOMXPlayer.h"


bool doClose = false;

void termination_handler(int signum)
{
	doClose = true;
}

void ofxOMXPlayer::exit()
{
	close();
	ofExit();
}
void ofxOMXPlayer::update(ofEventArgs& args)
{
	if (doClose) 
	{
		doClose = false;
		ofRemoveListener(ofEvents().update, this, &ofxOMXPlayer::update);
		exit();
	}
}
ofxOMXPlayer::ofxOMXPlayer()
{
	engine = NULL;
	isOpen = true;
	isTextureEnabled = false;
	ofAddListener(ofEvents().update, this, &ofxOMXPlayer::update);
}

void ofxOMXPlayer::loadMovie(string videoPath)
{
	settings.videoPath = videoPath;
	setup(settings);
}


//Structs that will describe the old action and the new action
//associated to the SIGINT signal (Ctrl+c from keyboard).
struct sigaction new_action, old_action;

bool ofxOMXPlayer::setup(ofxOMXPlayerSettings settings)
{
	
	this->settings = settings;
	
    //Set the handler in the new_action struct
    new_action.sa_handler = termination_handler;
	
    //Set to empty the sa_mask. It means that no signal is blocked
    // while the handler run.
    sigemptyset(&new_action.sa_mask);
	
    //Block the SEGTERM signal.
    // It means that while the handler run, the SIGTERM signal is ignored
    sigaddset(&new_action.sa_mask, SIGTERM);
	
    //Remove any flag from sa_flag. See documentation for flags allowed
    new_action.sa_flags = 0;
	
    //Read the old signal associated to SIGINT (keyboard, see signal(7))
    sigaction(SIGINT, NULL, &old_action);
	
    //If the old handler wasn't SIG_IGN (it's a handler that just
    // "ignore" the signal)
    if (old_action.sa_handler != SIG_IGN)
    {
        //Replace the signal handler of SIGINT with the one described by new_action
        sigaction(SIGINT,&new_action,NULL);
    }

	
	openEngine();
	
}
void ofxOMXPlayer::openEngine()
{
	if (engine) 
	{
		delete engine;
		engine = NULL;
	}
	engine = new ofxOMXPlayerEngine();
	engine->setup(settings);
	isTextureEnabled = engine->isTextureEnabled;
}

void ofxOMXPlayer::setPaused(bool doPause)
{
	if (engine) 
	{
		return engine->setPaused(doPause);
	}
}

bool ofxOMXPlayer::isPaused()
{
	if (engine) 
	{
		return engine->isPaused();
	}
	return false;
}

bool ofxOMXPlayer::isPlaying()
{
	if (engine) 
	{
		return engine->isPlaying();
	}
	return false;
}

int ofxOMXPlayer::getHeight()
{
	if (engine) 
	{
		return engine->getHeight();
	}
	return 0;
}

int ofxOMXPlayer::getWidth()
{
	if (engine) 
	{
		return engine->getWidth();
	}
	return 0;
}
double ofxOMXPlayer::getMediaTime()
{
	if (engine) 
	{
		return engine->getMediaTime();
	}
	return 0;
}
void ofxOMXPlayer::stepFrameForward()
{
	if (engine) 
	{
		engine->stepFrameForward();
	}
}

void ofxOMXPlayer::increaseVolume()
{
	if (engine) 
	{
		engine->increaseVolume();
	}
}
void ofxOMXPlayer::decreaseVolume()
{
	if (engine) 
	{
		engine->decreaseVolume();
	}
}

float ofxOMXPlayer::getDuration()
{
	if (engine) 
	{
		return engine->getDuration();
	}
	return 0;
}


void ofxOMXPlayer::setVolume(float volume)
{
	if (engine) 
	{
		engine->setVolume(volume);
	}
}

float ofxOMXPlayer::getVolume()
{
	if (engine) 
	{
		return engine->getVolume();
	}
	return 0;
}

GLuint ofxOMXPlayer::getTextureID()
{
	return GlobalEGLContainer::getInstance().textureID;
}


ofTexture & ofxOMXPlayer::getTextureReference()
{
	
	return GlobalEGLContainer::getInstance().texture;
}

void ofxOMXPlayer::saveImage(string imagePath)//default imagePath=""
{
	if(imagePath == "")
	{
		imagePath = ofGetTimestampString()+".png";
	}
	updatePixels();
	ofSaveImage(GlobalEGLContainer::getInstance().pixels, ofGetTimestampString()+".png");
	
}

void ofxOMXPlayer::updatePixels()
{
	
	GlobalEGLContainer::getInstance().updatePixels();
}

int ofxOMXPlayer::getCurrentFrame()
{
	if (engine) 
	{
		return engine->getCurrentFrame();
	}
	return 0;
}

int ofxOMXPlayer::getTotalNumFrames()
{
	if (engine) 
	{
		return engine->getTotalNumFrames();
	}
	return 0;
}

void ofxOMXPlayer::draw(float x, float y, float width, float height)
{
	if (!isTextureEnabled) 
	{
		return;
	}
	getTextureReference().draw(x, y, width, height);	
}

void ofxOMXPlayer::draw(float x, float y)
{
	if (!isTextureEnabled) 
	{
		return;
	}
	getTextureReference().draw(x, y);
}


bool hasTriedToKill = false;
void killSwitch(int sig) {
	if(!hasTriedToKill)
	{
		cout << "\n Hit Ctrl+C again to exit hard" << endl;
		hasTriedToKill = true;
	}else 
	{
		cout << "Detected multiple attempts to kill - exiting hard" << endl;	
		_Exit(0);
	}

}

void ofxOMXPlayer::close()
{	
	ofLogVerbose(__func__) << " isOpen: " << isOpen;
	if (!isOpen) 
	{
		return;
	}
	//signal(SIGINT,  &killSwitch);
	
	if(engine)
	{
		delete engine;
		engine = NULL;
	}
	GlobalEGLContainer::getInstance().destroyEGLImage();
	isOpen = false;
	//atexit(NULL);
	//ofxOMXPlayerInstance = NULL;
	//signal(SIGINT,  NULL);
	
}
ofxOMXPlayer::~ofxOMXPlayer()
{
	ofLogVerbose(__func__) << " isOpen: " << isOpen;
	close();
}