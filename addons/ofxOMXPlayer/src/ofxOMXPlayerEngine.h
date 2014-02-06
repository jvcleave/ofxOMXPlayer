#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"
#include "ofxOMXPlayerListener.h"

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




class ofxOMXPlayerEngine: public OMXThread
{
public:
	ofxOMXPlayerEngine();
	~ofxOMXPlayerEngine();
	
	bool setup(ofxOMXPlayerSettings settings);
	
	void		loadMovie();
	
	void		play();
	void		stop();
	
	
	float		getDuration();
	
	
	void		setVolume(float volume); // 0..1
	float		getVolume();
	
	
	
	
	void		setPaused(bool doPause);
	
	int			getCurrentFrame();
	int			getTotalNumFrames();
	
	
	int			getHeight();
	int			getWidth();
	
	bool		isPaused();
	bool		isPlaying();
		
	double		getMediaTime();
	bool		useHDMIForAudio;
	
	bool		isTextureEnabled;
	
	void		stepFrameForward();
	void		increaseVolume();
	void		decreaseVolume();
	
	void		addListener(ofxOMXPlayerListener* listener_);
	void		removeListener();

	//OMXThread inheritance
	void		Process();
	
	COMXStreamInfo			videoStreamInfo;
	COMXStreamInfo			audioStreamInfo;
	
private:
	
	COMXCore				omxCore;
	OMXClock				clock;
	
	OMXPacket*				packet;
	OMXPlayerVideo*			nonEglPlayer;
	OMXPlayerEGLImage*		eglPlayer;
	OMXPlayerVideoBase*		videoPlayer;
	OMXPlayerAudio*			audioPlayer;
	ofxOMXPlayerListener*	listener;
	OMXReader				omxReader;
	

	
	bool					hasVideo;
	bool					hasAudio;

	bool					openPlayer();
	bool					didVideoOpen;
	bool					didAudioOpen;
	bool					doLooping;
	int						videoWidth;
	int						videoHeight;
	
	string					moviePath;
	int						nFrames;
	bool					bPlaying;

	double					loop_offset;
	double					startpts;
	int						loopCounter;
	
	float					duration;
		
	void					onVideoEnd();
	void					onVideoLoop();
	double					previousLoopOffset;
};

