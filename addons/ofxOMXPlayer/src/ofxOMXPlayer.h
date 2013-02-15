#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"

extern "C" 
{
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
};

#include "RBP.h"
#include "OMXClock.h"
#include "OMXPlayerVideo.h"


class ofxOMXPlayer 
{
public:
	ofxOMXPlayer();
	void setup(string filepath);
	void update();
	void draw();
	string filepath;
	CRBP                  rbp;
	COMXCore              omxCore;
	OMXClock * clock;
	
	OMXPlayerVideo    omxPlayerVideo;
	OMXReader         omxReader;
	
	COMXStreamInfo    streamInfo;
	
	bool isMPEG;
	bool hasVideo;
	
	DllBcmHost        bcmHost;
	
	bool isReady;
	
	OMXPacket* packet;
	
	ofTexture textureSource;
	GLuint texture;
	
	
	
	int videoWidth;
	int videoHeight;
	
	EGLDisplay display;
	EGLContext context;
	EGLImageKHR eglImage;
	
	void generateEGLImage();
	
	void close();
};

