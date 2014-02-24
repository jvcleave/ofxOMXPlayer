#pragma once

#include "ofMain.h"
#include "ofxOMXPlayerSettings.h"
#include "ofxOMXPlayerListener.h"
#include "ofxOMXPlayerEngine.h"




class ofxOMXPlayer
{
public:
	ofxOMXPlayer();
	~ofxOMXPlayer();
	bool setup(ofxOMXPlayerSettings settings);
	
	void loadMovie(string videoPath);
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
	void saveImage(string imagePath="");
	void updatePixels();
	   
	void close();
	bool isOpen;
	
	COMXStreamInfo getVideoStreamInfo();
	COMXStreamInfo getAudioStreamInfo();

	void setNormalSpeed();
	void fastForward();
	void rewind();
	
	ofFbo fbo;
	ofTexture texture;
	EGLImageKHR eglImage;
	GLuint textureID;
	ofAppEGLWindow*		appEGLWindow;
	EGLDisplay			display;
	EGLContext			context;

	int videoWidth;
	int videoHeight;
	unsigned char * pixels;
	bool isExiting;
	
	void generateEGLImage(int videoWidth, int videoHeight);
	void destroyEGLImage();
	
private:
	
	void openEngine();
	void addExitHandler();
	void onUpdate(ofEventArgs& args);
	ofxOMXPlayerEngine* engine;
	ofxOMXPlayerSettings settings;
};