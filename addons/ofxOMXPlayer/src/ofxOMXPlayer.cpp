/*
 *  ofxOMXPlayer.cpp
 *  openFrameworksLib
 *
 *  Created by jason van cleave on 2/15/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */

#include "ofxOMXPlayer.h"

ofxOMXPlayer::ofxOMXPlayer()
{
	isReady = false;
	videoWidth = 0;
	videoHeight =0;
	isMPEG               = false;
	hasVideo           = false;
	packet = NULL;
	moviePath = "moviePath is undefined";
	clock = NULL;
	playerTex = NULL;
	pixels = NULL;
	m_Pause = false;
	
}
void ofxOMXPlayer::loadMovie(string filepath)
{
	moviePath = filepath; 
	rbp.Initialize();
	omxCore.Initialize();
	
	clock = new OMXClock(); 
	bool doDumpFormat = false;

	if(omxReader.Open(moviePath.c_str(), doDumpFormat))
	{
		
		ofLogVerbose() << "omxReader successfully opened moviePath : " << moviePath;
		hasVideo     = omxReader.VideoStreamCount();
		if (hasVideo) 
		{
			bool hasAudio = false; //not implemented yet
			if(clock->OMXInitialize(hasVideo, hasAudio))
			{
				ofLogVerbose() << "clock Init success";
				
				omxReader.GetHints(OMXSTREAM_VIDEO, streamInfo);
				videoWidth	= streamInfo.width;
				videoHeight	= streamInfo.height;
				
				generateEGLImage();
				bool didOpenVideo = omxPlayerVideo.Open(streamInfo, clock, eglImage);
				
				if (didOpenVideo) 
				{
					ofLogVerbose() << "Opened video!";
					isReady  = true;
					
				}else 
				{
					ofLogError() << "could not open video";
				}
				
			}else 
			{
				ofLogError() << "clock could not init";
			}
		}else 
		{
			ofLogError() << "No Video detected";
		}
	}else 
	{
		ofLogError() << "omxReader could not open file: " << moviePath;
	}
	
}
void ofxOMXPlayer::generateEGLImage()
{
	ofDisableArbTex();
	
	ofAppEGLWindow *appEGLWindow = (ofAppEGLWindow *) ofGetWindowPtr();
	display = appEGLWindow->getEglDisplay();
	context = appEGLWindow->getEglContext();
	ofLogVerbose() << "videoWidth: " << videoWidth;
	ofLogVerbose() << "videoHeight: " << videoHeight;
	
	tex.allocate(videoWidth, videoHeight, GL_RGBA);
	tex.getTextureData().bFlipTexture = true;
	tex.setTextureWrap(GL_REPEAT, GL_REPEAT);
	//textureSource.setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
	//textureSource.setTextureWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	textureID = tex.getTextureData().textureID;
	
	
	glEnable(GL_TEXTURE_2D);
	
	
	// setup first texture
	int dataSize = videoWidth * videoHeight * 4;
	
	GLubyte* pixelData = new GLubyte [dataSize];
	
	
    memset(pixelData, 0xff, dataSize);  // white texture, opaque
	
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, videoWidth, videoHeight, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
	
	delete[] pixelData;
	
	
	// Create EGL Image
	eglImage = eglCreateImageKHR(
								 display,
								 context,
								 EGL_GL_TEXTURE_2D_KHR,
								 (EGLClientBuffer)textureID,
								 0);
    glDisable(GL_TEXTURE_2D);
	if (eglImage == EGL_NO_IMAGE_KHR)
	{
		ofLogError()	<< "Create EGLImage FAIL";
		return;
	}
	else
	{
		ofLogVerbose()	<< "Create EGLImage PASS";
	}
	pixels = new ofPixels();
	pixels->allocate(videoWidth, videoHeight, OF_PIXELS_RGBA);
}

//---------------------------------------------------------------------------
//for getting a reference to the texture
ofTexture & ofxOMXPlayer::getTextureReference(){
	if(playerTex == NULL){
		return tex;
	}
	else{
		return *playerTex;
	}
}

void ofxOMXPlayer::update()
{
	if(!isReady)
	{
		return;
	}
	
	if(!packet)
	{
		packet = omxReader.Read();
	}
	
	if(packet && omxReader.IsActive(OMXSTREAM_VIDEO, packet->stream_index))
    {
		if(omxPlayerVideo.AddPacket(packet))
		{
			packet = NULL;
		}else 
		{
			OMXClock::OMXSleep(10);
		}
	}else 
	{
		if(packet)
		{
			omxReader.FreePacket(packet);
			packet = NULL;
		}
	}
}
//--------------------------------------------------------
void ofxOMXPlayer::play(){
	
}

//--------------------------------------------------------
void ofxOMXPlayer::stop(){
	
}

//---------------------------------------------------------------------------
void ofxOMXPlayer::setPaused(bool _bPause){
	m_Pause = _bPause;
	if(m_Pause)
	{
		omxPlayerVideo.SetSpeed(OMX_PLAYSPEED_PAUSE);
		clock->OMXPause();
	}else 
	{
		omxPlayerVideo.SetSpeed(OMX_PLAYSPEED_NORMAL);
		clock->OMXResume();
	}

			
}

//---------------------------------------------------------------------------
unsigned char * ofxOMXPlayer::getPixels(){
	if( pixels != NULL ){
		return pixels->getPixels();
	}
	return NULL;	
}

//---------------------------------------------------------------------------
ofPixelsRef ofxOMXPlayer::getPixelsRef(){
	if (pixels ==NULL) 
	{
		//TODO figure this out
		ofLogError() << "probably going to crash";
	}
	return *pixels;
}

//------------------------------------
void ofxOMXPlayer::draw(float _x, float _y, float _w, float _h)
{
	if(!isReady) return;
	getTextureReference().draw(_x, _y, _w, _h);	
}

//------------------------------------
void ofxOMXPlayer::draw(float _x, float _y)
{
	if(!isReady) return;
	getTextureReference().draw(_x, _y);
}

//----------------------------------------------------------
float ofxOMXPlayer::getWidth()
{
	return videoWidth;
}

//----------------------------------------------------------
float ofxOMXPlayer::getHeight()
{
	return videoHeight;
}

void ofxOMXPlayer::close()
{
	if(isReady) 
	 {
		 clock->OMXStop();
		 clock->OMXStateIdle();
		 omxPlayerVideo.Close();
	 }
	 if(packet)
	 {
		 omxReader.FreePacket(packet);
		 packet = NULL;
	 }	
		omxReader.Close();

	 if (eglImage !=NULL) 
	 {
		 eglDestroyImageKHR(display, eglImage);
	 }

	omxCore.Deinitialize();
	rbp.Deinitialize();
}
