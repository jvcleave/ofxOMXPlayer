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
		bool		isFrameNew();
		bool isTextureEnabled;

		ofTexture&	getTextureReference();
		GLuint		getTextureID();
		int			getHeight();
		int			getWidth();

		void		draw(float x, float y, float w, float h);
        void        setDisplayRectForNonTexture(float x, float y, float width, float height);
		void		draw(float x=0, float y=0);


		double		getMediaTime();
		void		stepFrameForward();
		void		increaseVolume();
		void		decreaseVolume();

		float		getDuration();


		void		setVolume(float volume); // 0..1
		float		getVolume();


		int			getCurrentFrame();
		int			getTotalNumFrames();
		void		togglePause();
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
		void restartMovie();
	
		unsigned char * getPixels();
		ofxOMXPlayerSettings settings;
		string getInfo();
	
	private:

		bool openEngine();
		void addExitHandler();
		void onUpdateDuringExit(ofEventArgs& args);
		ofxOMXPlayerEngine* engine;
		

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

		void generateEGLImage(int videoWidth_, int videoHeight_);
		void destroyEGLImage();
	
		bool hasNewFrame;
		int prevFrame;
		void onUpdate(ofEventArgs& args);
		bool doRestart;

};