#pragma once

#include "ofMain.h"

extern "C" 
{
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
};

#include "RBP.h"
#include "OMXClock.h"
#include "OMXPlayerVideo.h"
#include "OMXPlayerAudio.h"


class ofxOMXVideoPlayer : public ofThread
{
public:
	ofxOMXVideoPlayer();
	void loadMovie(string filepath);
	void update();
	
	void 				play();
	void 				stop();
	
	float				duration;
	float 				getDuration();
	
	void 				setPosition(float pct);
//	void 				setVolume(float volume); // 0..1
	
	
	void 				setPaused(bool doPause);
	
	int					getCurrentFrame();
	int					getTotalNumFrames();
	
	
	float 				getHeight();
	float 				getWidth();
	
	bool				isPaused();
	bool				isPlaying();
		

	int videoWidth;
	int videoHeight;
	
	void openPlayer();
	void close();
	double getMediaTime();
	bool doLooping;
	void threadedFunction();
private:
	
	CRBP rbp;
	COMXCore omxCore;
	OMXClock * clock;
	
	OMXPlayerVideo videoPlayer;
	OMXPlayerAudio m_player_audio;

	OMXReader omxReader;
	
	COMXStreamInfo streamInfo;
	COMXStreamInfo audioStreamInfo;
	bool isMPEG;
	bool hasVideo;
	bool hasAudio;
	DllBcmHost bcmHost;
	
	
	OMXPacket* packet;
	
	string moviePath;
	bool bPlaying;
	int nFrames;
};

