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
#include "OMXCore.h"

class GlobalEGLContainer
{
public:
	static GlobalEGLContainer& getInstance()
	{
		static GlobalEGLContainer    instance;
		return instance;
	}
	ofTexture* texture;
	EGLImageKHR eglImage;
	GLuint textureID;
	OMX_BUFFERHEADERTYPE* eglBuffer;

private:
	GlobalEGLContainer() {
		texture = NULL;
		eglImage = NULL;
		textureID = 0;
		eglBuffer = NULL;
	};
	GlobalEGLContainer(GlobalEGLContainer const&);              // Don't Implement.
	void operator=(GlobalEGLContainer const&);					// Don't implement
};