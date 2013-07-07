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
#include "OMXPlayerEGLImage.h"
#include "OMXPlayerVideo.h"
#include "OMXPlayerAudio.h"

struct ofxOMXPlayerSettings 
{
	ofxOMXPlayerSettings()
	{
		videoPath = "";
		useHDMIForAudio = true;
		enableTexture = true;
	}
	string videoPath;
	bool enableTexture;
	bool useHDMIForAudio;
	
	
};


class ofxOMXPlayer : public ofThread
{
public:
	ofxOMXPlayer();
	void setup(ofxOMXPlayerSettings settings_);
	ofxOMXPlayerSettings settings;
	
	void			loadMovie();
	
	void			play();
	void			stop();
	
	float			duration;
	float			getDuration();
	
	void			setPosition(float pct);
	void			setVolume(float volume); // 0..1
	float			getVolume();
	ofTexture &		getTextureReference();
	void			draw(float x, float y, float w, float h);
	void			draw(float x, float y);
	
	void			setPaused(bool doPause);
	
	int				getCurrentFrame();
	int				getTotalNumFrames();
	
	
	float			getHeight();
	float			getWidth();
	
	bool			isPaused();
	bool			isPlaying();
		
	GLuint			textureID;

	int				videoWidth;
	int				videoHeight;
	

	EGLImageKHR		eglImage;
		
	void			openPlayer();
	double			getMediaTime();
	void			close(ofEventArgs & a);
	bool			doVideoDebugging;
	bool			doLooping;
	void			threadedFunction();

	bool			isTextureEnabled;
	bool			didVideoOpen;
	bool			didAudioOpen;
	void			stepFrameForward();
	void			increaseVolume();
	void			decreaseVolume();
private:
	
	CRBP				rbp;
	COMXCore			omxCore;
	OMXClock*			clock;
	DllBcmHost			bcmHost;
	
	OMXPlayerVideo*			nonEglPlayer;
	OMXPlayerEGLImage*		eglPlayer;
	OMXPlayerVideoBase*		videoPlayer;
	OMXPlayerAudio*			audioPlayer;
	
	OMXReader			omxReader;
	
	COMXStreamInfo		videoStreamInfo;
	COMXStreamInfo		audioStreamInfo;
	
	bool				isMPEG;
	bool				hasVideo;
	bool				hasAudio;
	bool				isBufferEmpty;

	
	
	
	OMXPacket*			packet;
	
	ofTexture			tex;
	ofPixelFormat		internalPixelFormat;
	string				moviePath;
	int					nFrames;
	bool				bPlaying;
	EGLDisplay			display;
	EGLContext			context;
	
	
	double				loop_offset;
	double				startpts;
	int					loopCounter;
	void				generateEGLImage();
};

