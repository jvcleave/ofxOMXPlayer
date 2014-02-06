/*
 *  ofxOMXPlayer.cpp
 *
 *  Created by jason van cleave on 9/8/13.
 *
 */

#include "ofxOMXPlayer.h"

ofxOMXPlayer::ofxOMXPlayer()
{
	engine = NULL;
	isOpen = true;
	isTextureEnabled = false;
	
}

void ofxOMXPlayer::loadMovie(string videoPath)
{
	settings.videoPath = videoPath;
	setup(settings);
}


bool ofxOMXPlayer::setup(ofxOMXPlayerSettings settings)
{	
	this->settings = settings;
	addExitHandler();
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


COMXStreamInfo ofxOMXPlayer::getVideoStreamInfo()
{
	
	COMXStreamInfo videoInfo;
	if (engine) 
	{
		videoInfo = engine->videoStreamInfo;
		
	}else
	{
		ofLogError(__func__) << "No engine avail - info returned is invalid";
	}
	return videoInfo;
}

COMXStreamInfo ofxOMXPlayer::getAudioStreamInfo()
{
	COMXStreamInfo audioInfo;
	if (engine) 
	{
		audioInfo = engine->audioStreamInfo;

	}else
	{
		ofLogError(__func__) << "No engine avail - info returned is invalid";
	}
	return audioInfo;
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

void ofxOMXPlayer::close()
{	
	ofLogVerbose(__func__) << " isOpen: " << isOpen;
	if (!isOpen) 
	{
		return;
	}
	ofRemoveListener(ofEvents().update, this, &ofxOMXPlayer::onUpdate);
	
	if(engine)
	{
		delete engine;
		engine = NULL;
	}
	GlobalEGLContainer::getInstance().destroyEGLImage();
	isOpen = false;
	
}
ofxOMXPlayer::~ofxOMXPlayer()
{
	close();
}

bool doExit = false;
void termination_handler(int signum)
{
	doExit = true;
}

void ofxOMXPlayer::onUpdate(ofEventArgs& args)
{
	if (doExit) 
	{
		ofLogVerbose(__func__) << " EXITING VIA SIGNAL";
		doExit = false;
		close();
		ofExit();
	}
}

void ofxOMXPlayer::addExitHandler()
{
	
	//http://stackoverflow.com/questions/11465148/using-sigaction-c-cpp
	//Structs that will describe the old action and the new action
	//associated to the SIGINT signal (Ctrl+c from keyboard).
	struct sigaction new_action, old_action;
	
	//Set the handler in the new_action struct
    new_action.sa_handler = termination_handler;
	
    //Set to empty the sa_mask. It means that no signal is blocked while the handler run.
    sigemptyset(&new_action.sa_mask);
	
    //Block the SEGTERM signal.
    // It means that while the handler run, the SIGTERM signal is ignored
    sigaddset(&new_action.sa_mask, SIGTERM);
	
    //Remove any flag from sa_flag. See documentation for flags allowed
    new_action.sa_flags = 0;
	
    //Read the old signal associated to SIGINT (keyboard, see signal(7))
    sigaction(SIGINT, NULL, &old_action);
	
    //If the old handler wasn't SIG_IGN (it's a handler that just "ignore" the signal)
    if (old_action.sa_handler != SIG_IGN)
    {
        //Replace the signal handler of SIGINT with the one described by new_action
        sigaction(SIGINT,&new_action,NULL);
    }
	ofAddListener(ofEvents().update, this, &ofxOMXPlayer::onUpdate);
}

