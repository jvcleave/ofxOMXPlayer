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
	void loadMovie(string filepath);
	void update();
	
	void 				play();
	void 				stop();
	
	//bool 				isFrameNew();
	unsigned char * 	getPixels();
	ofPixelsRef			getPixelsRef();
	float 				getPosition();
	int 				getSpeed();
	float				duration;
	float 				getDuration();
//	bool				getIsMovieDone();
	
	void 				setPosition(float pct);
//	void 				setVolume(float volume); // 0..1
	void 				setLoopState(ofLoopType state);
	ofLoopType			getLoopState();
	int					speed;
	void   				setSpeed(int rate);
//	void				setFrame(int frame);  // frame 0 = first frame...
	
	ofTexture &			getTextureReference();
	void 				draw(float x, float y, float w, float h);
	void 				draw(float x, float y);

	
	void				setAnchorPercent(float xPct, float yPct);	//set the anchor as a percentage of the image width/height ( 0.0-1.0 range )
	void				setAnchorPoint(float x, float y);				//set the anchor point in pixels
	void				resetAnchor();								//resets the anchor to (0, 0)
	
	void 				setPaused(bool bPause);
	
	int					getCurrentFrame();
	int					getTotalNumFrames();
	
//	void				firstFrame();
//	void				nextFrame();
//	void				previousFrame();
	
	float 				getHeight();
	float 				getWidth();
	
	bool				isPaused();
//	bool				isLoaded();
	bool				isPlaying();
	
	CRBP                  rbp;
	COMXCore              omxCore;
	OMXClock * clock;
	
	OMXPlayerVideo    omxPlayerVideo;
	OMXReader         omxReader;
	
	COMXStreamInfo    streamInfo;
	
	bool isMPEG;
	bool hasVideo;
	
	DllBcmHost        bcmHost;
	
	
	OMXPacket* packet;
	
	GLuint textureID;
	bool bPaused;
	bool bPlaying;
	
	int videoWidth;
	int videoHeight;
	
	EGLDisplay display;
	EGLContext context;
	EGLImageKHR eglImage;
	
	string getVideoDebugInfo();
	void generateEGLImage();
	void openPlayer();
	double getMediaTime();
	ofPixels* pixels;
	void close();
	bool doVideoDebugging;
	bool doLooping;
private:
	
	ofTexture tex;
	ofTexture * playerTex; // a seperate texture that may be optionally implemented by the player to avoid excessive pixel copying.
	ofPixelFormat internalPixelFormat;
	string moviePath;
	int nFrames;
};

