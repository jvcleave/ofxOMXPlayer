/*
 *  ofxOMXPlayer.cpp
 *  openFrameworksRPi
 *
 *  Created by jason van cleave on 9/8/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */

#include "ofxOMXPlayer.h"


ofxOMXPlayer::ofxOMXPlayer()
{
	engine = NULL;
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

ofxOMXPlayer::~ofxOMXPlayer()
{
	ofLogVerbose() << "~ofxOMXPlayer";
	signal(SIGINT,  &killSwitch);
	if(engine)
	{
		delete engine;
		engine = NULL;
	}
	signal(SIGINT,  NULL);
}