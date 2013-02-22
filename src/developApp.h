#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "ofxOMXVideoPlayer.h"

#include "PipeReader.h"

class developApp : public ofBaseApp, public ofxPipeListener{
	
public:
	
	void setup();
	void update();
	void draw();
	void exit();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);	
	
	ofxOMXPlayer omxPlayer;
	ofxOMXVideoPlayer omxVideoPlayer;
	ofShader shader;
	ofFbo fbo;
	void updateFbo();
	
	bool doShader;
	bool doPause;
	bool doTextures;
	
	void initOMXPlayer();
	void updateTexturePlayer();
	//allows key commands via Shell via ofxPipeListener
	PipeReader pipeReader;
	void onCharacterReceived(ofxPipeListenerEventData& e);
	
	void initVideoPlayer();
	string videoPath;
};

