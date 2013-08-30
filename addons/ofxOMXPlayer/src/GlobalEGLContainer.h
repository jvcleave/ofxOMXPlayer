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
private:
	GlobalEGLContainer() {
		textureID = 0;
		appEGLWindow = NULL;
		hasGenerated = false;
		doLooping = false;
	};
	GlobalEGLContainer(GlobalEGLContainer const&);              // Don't Implement.
	void operator=(GlobalEGLContainer const&);					// Don't implement
};