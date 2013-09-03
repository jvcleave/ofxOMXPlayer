/*
 *  GlobalEGLContainer.h
 *  openFrameworksLib
 *
 *  Created by jason van cleave on 8/28/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */
#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"
#include <IL/OMX_Core.h>

class GlobalEGLContainer
{
public:
	static GlobalEGLContainer& getInstance()
	{
		static GlobalEGLContainer    instance;
		return instance;
	}
	ofTexture texture;
	EGLImageKHR eglImage;
	GLuint textureID;
	ofAppEGLWindow*		appEGLWindow;
	EGLDisplay			display;
	EGLContext			context;
	bool hasGenerated;
	bool doLooping;
	int videoWidth;
	int videoHeight;
	
	void generateEGLImage(int videoWidth_, int videoHeight_)
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
		
		if (hasGenerated) 
		{
			
			if (!needsRegeneration) 
			{
				return;
			}else 
			{
				destroyEGLImage();
			}
		}
		
		if (appEGLWindow == NULL) 
		{
			appEGLWindow = (ofAppEGLWindow *) ofGetWindowPtr();
		}
		
		if (!appEGLWindow) 
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
		
		
		if (texture.isAllocated()) 
		{
			texture.clear();
		}
		
		texture.allocate(videoWidth, videoHeight, GL_RGBA);
		texture.getTextureData().bFlipTexture = true;
		texture.setTextureWrap(GL_REPEAT, GL_REPEAT);
		textureID = texture.getTextureData().textureID;
		
		
		ofLogVerbose(__func__) << "textureID: " << textureID;
		ofLogVerbose(__func__) << "tex.isAllocated(): " << texture.isAllocated();
		
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
		}
		else
		{
			ofLogVerbose()	<< "Create EGLImage PASS";
			hasGenerated = true;
		}
		
	}
private:
	GlobalEGLContainer() 
	{
		textureID = 0;
		videoWidth =0;
		videoHeight = 0;
		appEGLWindow = NULL;
		eglImage = NULL;
		context = NULL;
		display = NULL;
		hasGenerated = false;
		doLooping = false;
	};
	
	void destroyEGLImage()
	{
		/*if (eglImage) 
		{
			if (eglDestroyImageKHR(display, eglImage)) 
			{
				ofLogVerbose(__func__) << "eglDestroyImageKHR PASS";
			}
			else
			{
				ofLogError(__func__) << "eglDestroyImageKHR FAIL";
			}
			eglImage = NULL;
		}*/
		
	}
	
	~GlobalEGLContainer()
	{
		destroyEGLImage();
	};
	GlobalEGLContainer(GlobalEGLContainer const&);
	void operator=(GlobalEGLContainer const&);
};