#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"


extern "C" 
{
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
};

#include "OMXThread.h"
#include "OMXClock.h"
#include "OMXPlayerEGLImage.h"
#include "OMXPlayerVideo.h"
#include "OMXPlayerAudio.h"


class ofxOMXPlayerListenerEventData
{
public:
	ofxOMXPlayerListenerEventData(void* listener_)
	{
		listener = listener_;
	}
	void* listener;
};


class ofxOMXPlayerListener
{
public:
	virtual void onVideoEnd(ofxOMXPlayerListenerEventData& e) = 0;
};


struct ofxOMXPlayerSettings 
{
	ofxOMXPlayerSettings()
	{
		videoPath = "";
		
		useHDMIForAudio = true;
		enableTexture = true;
		enableLooping = true;
		listener	  = NULL;
	}
	string videoPath;
	bool enableTexture;
	bool useHDMIForAudio;
	bool enableLooping;
	ofxOMXPlayerListener* listener;
	/*
		To use HDMI Audio you may need to add the below line to /boot/config.txt and reboot
	 
		hdmi_drive=2
	 
		see http://elinux.org/RPiconfig for more details
	 */
	
};


class ofxOMXPlayer: public OMXThread
{
public:
	ofxOMXPlayer();
	~ofxOMXPlayer();
	
	bool setup(ofxOMXPlayerSettings settings);
	
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
		

	int				videoWidth;
	int				videoHeight;
	GLuint			textureID;

		
	bool			openPlayer();
	double			getMediaTime();
	bool			useHDMIForAudio;
	
	bool			isTextureEnabled;
	bool			didVideoOpen;
	bool			didAudioOpen;
	void			stepFrameForward();
	void			increaseVolume();
	void			decreaseVolume();
	
	void			addListener(ofxOMXPlayerListener* listener_);
	void			removeListener();

	void						Process();
	void						Lock();
	void						UnLock();
	
private:
	
	COMXCore				omxCore;
	OMXClock*				clock;
	
	OMXPlayerVideo*			nonEglPlayer;
	OMXPlayerEGLImage*		eglPlayer;
	OMXPlayerVideoBase*		videoPlayer;
	OMXPlayerAudio*			audioPlayer;
	ofxOMXPlayerListener*	listener;
	OMXReader				omxReader;
	
	COMXStreamInfo			videoStreamInfo;
	COMXStreamInfo			audioStreamInfo;
	
	bool					isMPEG;
	bool					hasVideo;
	bool					hasAudio;
	bool					isBufferEmpty;

	
	
	
	OMXPacket*				packet;
	

	
	ofPixelFormat			internalPixelFormat;
	string					moviePath;
	int						nFrames;
	bool					bPlaying;

	
	
	double					loop_offset;
	double					startpts;
	int						loopCounter;
	
	
	void					onVideoEnd();
};

