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

	textureID = 0;
	videoWidth =0;
	videoHeight = 0;
	appEGLWindow = NULL;
	eglImage = NULL;
	context = NULL;
	display = NULL;
	pixels = NULL;
}

void ofxOMXPlayer::updatePixels()
{
	fbo.begin(false);
	ofClear(0, 0, 0, 0);
	texture.draw(0, 0);
	glReadPixels(0,0,videoWidth, videoHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	fbo.end();
}

unsigned char * ofxOMXPlayer::getPixels()
{
	return pixels;
}

void ofxOMXPlayer::generateEGLImage(int videoWidth_, int videoHeight_)
{
	bool needsRegeneration = false;
	if (videoWidth != videoWidth_)
	{
		needsRegeneration = true;
		videoWidth = videoWidth_;
	}
	if (videoHeight != videoHeight_)
	{
		needsRegeneration = true;
		videoHeight = videoHeight_;
	}
	if (!fbo.isAllocated())
	{
		needsRegeneration = true;
	}
	else
	{
		if (fbo.getWidth() != videoWidth && fbo.getHeight() != videoHeight)
		{
			needsRegeneration = true;
		}
	}
	if (!texture.isAllocated())
	{
		needsRegeneration = true;
	}
	else
	{
		if (texture.getWidth() != videoWidth && texture.getHeight() != videoHeight)
		{
			needsRegeneration = true;
		}
	}

	if(!needsRegeneration)
	{
		ofLogVerbose(__func__) << "NO CHANGES NEEDED - RETURNING EARLY";
		return;
	}

	if (appEGLWindow == NULL)
	{
		appEGLWindow = (ofAppEGLWindow *) ofGetWindowPtr();
	}

	if (appEGLWindow == NULL)
	{
		ofLogError(__func__) << "appEGLWindow is NULL - RETURNING";
		return;
	}
	if (display == NULL)
	{
		display = appEGLWindow->getEglDisplay();
	}
	if (context == NULL)
	{
		context = appEGLWindow->getEglContext();
	}

	if (needsRegeneration)
	{
		ofFbo::Settings fboSettings;
		fboSettings.width = videoWidth;
		fboSettings.height = videoHeight;
		fboSettings.wrapModeVertical = GL_REPEAT;	// GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER etc.
		fboSettings.wrapModeHorizontal = GL_REPEAT; // GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER etc.
		//int		wrapModeHorizontal;		// GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER etc.
		//int		wrapModeVertical;		// GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER etc.

		fbo.allocate(fboSettings);
	}

	if (needsRegeneration)
	{
		texture.allocate(videoWidth, videoHeight, GL_RGBA);
		//Video renders upside down and backwards when Broadcom proprietary tunnels are enabled
		//may be resolved in future firmare
		//https://github.com/raspberrypi/firmware/issues/176

		if (settings.doFlipTexture)
		{
			texture.getTextureData().bFlipTexture = true;
		}
		texture.setTextureWrap(GL_REPEAT, GL_REPEAT);
		textureID = texture.getTextureData().textureID;
	}


	ofLogVerbose(__func__) << "textureID: " << textureID;
	ofLogVerbose(__func__) << "tex.isAllocated(): " << texture.isAllocated();

	glEnable(GL_TEXTURE_2D);

	// setup first texture
	int dataSize = videoWidth * videoHeight * 4;

	if (pixels && needsRegeneration)
	{
		delete[] pixels;
		pixels = NULL;
	}

	if (pixels == NULL)
	{
		pixels = new unsigned char[dataSize];
	}

	//memset(pixels, 0xff, dataSize);  // white texture, opaque

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, videoWidth, videoHeight, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, pixels);


	if (eglImage && needsRegeneration)
	{
		destroyEGLImage();
	}

	// Create EGL Image
	eglImage = eglCreateImageKHR(
	               display,
	               context,
	               EGL_GL_TEXTURE_2D_KHR,
	               (EGLClientBuffer)textureID,
	               NULL);
	glDisable(GL_TEXTURE_2D);
	if (eglImage == EGL_NO_IMAGE_KHR)
	{
		ofLogError()	<< "Create EGLImage FAIL <---------------- :(";
	}
	else
	{
		ofLogVerbose()	<< "Create EGLImage PASS <---------------- :)";

	}
}

void ofxOMXPlayer::destroyEGLImage()
{

	if (eglImage)
	{
		if (eglDestroyImageKHR(display, eglImage))
		{
			ofLogVerbose(__func__) << "eglDestroyImageKHR PASS <---------------- :)";
		}
		else
		{
			ofLogError(__func__) << "eglDestroyImageKHR FAIL <---------------- :(";
		}
		eglImage = NULL;
	}

}




void ofxOMXPlayer::setNormalSpeed()
{
	engine->setNormalSpeed();
}


void ofxOMXPlayer::rewind()
{
	engine->rewind();
}
void ofxOMXPlayer::fastForward()
{
	/*if(!m_av_clock)
		return;

	m_omx_reader.SetSpeed(iSpeed);

	// flush when in trickplay mode
	if (TRICKPLAY(iSpeed) || TRICKPLAY(m_av_clock->OMXPlaySpeed()))
		FlushStreams(DVD_NOPTS_VALUE);

	m_av_clock->OMXSetSpeed(iSpeed);*/
	engine->fastForward();

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

	//
	engine = new ofxOMXPlayerEngine();
	bool setupPassed = engine->setup(settings);
	if (setupPassed)
	{
		settings = engine->omxPlayerSettings;

		if (settings.enableTexture)
		{
			isTextureEnabled = settings.enableTexture;
			generateEGLImage(settings.videoWidth, settings.videoHeight);
			engine->eglImage = eglImage;
		}
		else
		{
			videoWidth	= settings.videoWidth;
			videoHeight = settings.videoHeight;
		}

		engine->openPlayer();
	}
	else
	{
		ofLogError(__func__) << "engine->setup FAIL";
	}


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
	return videoHeight;
}

int ofxOMXPlayer::getWidth()
{
	return videoWidth;
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

	return textureID;
}


ofTexture& ofxOMXPlayer::getTextureReference()
{

	return texture;
}

void ofxOMXPlayer::saveImage(string imagePath)//default imagePath=""
{
	if(imagePath == "")
	{
		imagePath = ofGetTimestampString()+".png";
	}
	updatePixels();
	//TODO ofSaveImage(GlobalEGLContainer::getInstance().pixels, ofGetTimestampString()+".png");

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

	}
	else
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

	}
	else
	{
		ofLogError(__func__) << "No engine avail - info returned is invalid";
	}
	return audioInfo;
}


void ofxOMXPlayer::draw(float x, float y, float width, float height)
{
	if (!texture.isAllocated())
	{
		return;
	}
	texture.draw(x, y, width, height);
}

void ofxOMXPlayer::draw(float x, float y)
{
	if (!texture.isAllocated())
	{
		return;
	}
	texture.draw(x, y);
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
		if(engine)
		{
			engine->startExit();
		}

		doExit = false;
		close();
		destroyEGLImage();
		if (pixels)
		{
			delete[] pixels;
			pixels = NULL;
		}
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

