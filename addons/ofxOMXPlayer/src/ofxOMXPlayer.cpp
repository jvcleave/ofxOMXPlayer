/*
 *  ofxOMXPlayer.cpp
 *  openFrameworksRPi
 *
 *  Created by jason van cleave on 9/8/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */

#include "ofxOMXPlayer.h"
ofxOMXPlayer* ofxOMXPlayerInstance;


/*void ofxOMXPlayer::exit()
{
	if (ofxOMXPlayerInstance) 
	{
		ofxOMXPlayerInstance->close();
	}
	
}*/

void onSIGINTHandler(int sig) 
{
	cout << "\n onSIGINTHandler" << endl;
	ofLogVerbose() << "IS MAIN THREAD: " << ofThread::isMainThread();
	signal(SIGINT,  NULL);
	//GlobalEGLContainer::getInstance().appEGLWindow->lock();
	//GlobalEGLContainer::getInstance().appEGLWindow->waitForThread(true);
	//GlobalEGLContainer::getInstance().appEGLWindow->sleep(2000);
	//
	//OMXClock::OMXSleep(2000);
	//ofxOMXPlayerInstance->engine->Lock();
	//ofxOMXPlayerInstance->engine->StopThread();
	ofxOMXPlayerInstance->close();
	ofxOMXPlayerInstance = NULL;
	raise(SIGINT);
	//exit(0);
	
	//raise(SIGABRT);
	//ofExit(0);
	//ofxOMXPlayerInstance->close();
	//GlobalEGLContainer::getInstance().isExiting = true;
	//signal(SIGINT,  SIG_DFL);
	//ofGetAppPtr()->exit();
	//ofxOMXPlayerInstance->engine->StopThread();
	
	//delete ofxOMXPlayerInstance->engine;
	//ofxOMXPlayerInstance->engine = NULL;
	//GlobalEGLContainer::getInstance().appEGLWindow->stopThread();
	//ofEvents().disable();
	//exit(0);
	//raise(SIGABRT);
	
	//signal(SIGINT,  NULL);
	//ofGetAppPtr()->exit();
	//ofxOMXPlayerInstance->close();
	//exit(0);
	//signal(SIGINT,  NULL);
	
	//ofExit(0);
	//GlobalEGLContainer::getInstance().appEGLWindow->yield();
	//ofxOMXPlayer::exit();
	//GlobalEGLContainer::getInstance().appEGLWindow->unlock();
	//ofxOMXPlayerInstance->close();
	//_Exit(0);
	//ofNotifyExit();
	//signal(SIGINT,  SIG_DFL);
	//
	//ofExit();
	//raise(SIGABRT);
	
}

void ofxOMXPlayerExit()
{
	cout << "ofxOMXPlayerExit" << endl;
	
	
}
bool hasThrownSig = false;

void ofxOMXPlayer::onUpdate(ofEventArgs & args)
{
	if (ofGetElapsedTimeMillis() > 7000 && !hasThrownSig) 
	{
		hasThrownSig = true;
		onSIGINTHandler(2);
	}
}

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
	
	ofxOMXPlayerInstance = NULL;
	ofxOMXPlayerInstance = this;
	this->settings = settings;
	signal(SIGINT,  NULL);
	signal(SIGINT,  &onSIGINTHandler);
	//atexit(ofxOMXPlayerExit);
	//ofAddListener(ofEvents().update, this, &ofxOMXPlayer::onUpdate);
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