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
	filepath = "NO FILENAME";
	clock = NULL;
	
}
void ofxOMXPlayer::setup(string filepath)
{
	this->filepath = filepath; 
	rbp.Initialize();
	omxCore.Initialize();
	
	clock = new OMXClock(); 
	bool doDumpFormat = false;

	if(omxReader.Open(filepath.c_str(), doDumpFormat))
	{
		
		ofLogVerbose() << "omxReader successfully opened: " << filepath;
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
		ofLogError() << "omxReader could not open file: " << filepath;
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
	
	textureSource.allocate(videoWidth, videoHeight, GL_RGBA);
	textureSource.getTextureData().bFlipTexture = true;
	textureSource.setTextureWrap(GL_REPEAT, GL_REPEAT);
	//textureSource.setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
	//textureSource.setTextureWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	texture = textureSource.getTextureData().textureID;
	
	
	glEnable(GL_TEXTURE_2D);
	
	
	// setup first texture
	int dataSize = videoWidth * videoHeight * 4;
	
	GLubyte* pixelData = new GLubyte [dataSize];
	
	
    memset(pixelData, 0xff, dataSize);  // white texture, opaque
	
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, videoWidth, videoHeight, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
	
	delete[] pixelData;
	
	
	// Create EGL Image
	eglImage = eglCreateImageKHR(
								 display,
								 context,
								 EGL_GL_TEXTURE_2D_KHR,
								 (EGLClientBuffer)texture,
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


void ofxOMXPlayer::draw()
{
	if(!isReady)
	{
		return;
	}
	
	textureSource.draw(0, 0, ofGetWidth(), ofGetHeight());
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
