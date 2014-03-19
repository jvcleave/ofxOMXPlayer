#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "ConsoleListener.h"


class playlistApp : public ofBaseApp, public ofxOMXPlayerListener, public SSHKeyListener{

	public:

		void setup();
		void update();
		void draw();
	
	
		void keyPressed(int key);
		ofxOMXPlayer omxPlayer;
	
		void onVideoEnd(ofxOMXPlayerListenerEventData& e);
		void onVideoLoop(ofxOMXPlayerListenerEventData& e){ /*empty*/ };

		
		vector<ofFile> files;
		int videoCounter;
	
		void onCharacterReceived(SSHKeyListenerEventData& e);
		ConsoleListener consoleListener;
		ofxOMXPlayerSettings settings;
	
	void loadNextMovie();
	
	ofShader shader;
	ofFbo fbo;
	void loadShader();
	bool doShader;
	bool doPixels;
	ofTexture pixelOutput;
	void check_gl_error(const char *file, int line) {
		GLenum err (glGetError());
		//ofLogVerbose(__func__) << " ";
		while(err!=GL_NO_ERROR) {
			string error;
			
			switch(err) {
				case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
				case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
				case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
				case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
				case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
			}
			
			ofLogError("GL_") << error << " - "<< file << ":" <<line;
			err=glGetError();
		}
	}
};

