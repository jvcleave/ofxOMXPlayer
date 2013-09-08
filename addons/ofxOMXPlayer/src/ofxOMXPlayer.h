#pragma once

#include "ofMain.h"
#include "ofxOMXPlayerListener.h"
#include "ofxOMXPlayerEngine.h"




class ofxOMXPlayer
{
public:
	ofxOMXPlayer();
	~ofxOMXPlayer();
	bool setup(ofxOMXPlayerSettings settings);
	
	
	bool		isPaused();
	bool		isPlaying();
	
	bool isTextureEnabled;
	
	ofTexture&	getTextureReference();
	GLuint		getTextureID();
	int			getHeight();
	int			getWidth();
	
	void		draw(float x, float y, float w, float h);
	void		draw(float x, float y);
	
	
	double		getMediaTime();
	void		stepFrameForward();
	void		increaseVolume();
	void		decreaseVolume();
	
	float		getDuration();
	
	
	void		setVolume(float volume); // 0..1
	float		getVolume();
	
	
	int			getCurrentFrame();
	int			getTotalNumFrames();
								  
	void		setPaused(bool doPause);					
	ofxOMXPlayerEngine* engine;

};