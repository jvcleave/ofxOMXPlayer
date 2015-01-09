#include "shaderApp.h"


void shaderApp::onCharacterReceived(KeyListenerEventData& e)
{
	keyPressed((int)e.character);
}


void shaderApp::onVideoEnd(ofxOMXPlayerListenerEventData& e)
{
	ofLogVerbose(__func__) << "at: " << ofGetElapsedTimeMillis();
}
void shaderApp::onVideoLoop(ofxOMXPlayerListenerEventData& e)
{
	ofLogVerbose(__func__) << "at: " << ofGetElapsedTimeMillis();
}


//--------------------------------------------------------------
void shaderApp::setup()
{
	consoleListener.setup(this);
	ofHideCursor();
		
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);
	
	ofxOMXPlayerSettings settings;
	settings.videoPath = videoPath;
	settings.listener = this; //this app extends ofxOMXPlayerListener so it will receive events
	
	
	shader.load("shaderExample");
	
	fbo.allocate(ofGetWidth(), ofGetHeight());
	fbo.begin();
		ofClear(0, 0, 0, 0);
	fbo.end();	omxPlayer.setup(settings);
	
}



//--------------------------------------------------------------
void shaderApp::update()
{
	fbo.begin();
		ofClear(0, 0, 0, 0);
		shader.begin();
			shader.setUniformTexture("tex0", omxPlayer.getTextureReference(), omxPlayer.getTextureID());
			shader.setUniform1f("time", ofGetElapsedTimef());
			shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
			omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
		shader.end();
	fbo.end();
}

//--------------------------------------------------------------
void shaderApp::draw(){
	
	fbo.draw(0, 0);
	ofDrawBitmapStringHighlight(omxPlayer.getInfo(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
	
}

//--------------------------------------------------------------
void shaderApp::keyPressed  (int key){
	 
	ofLog(OF_LOG_VERBOSE, "%c keyPressed", key);
	switch (key) 
	{
		case 'p':
		{
			ofLogVerbose() << "pause: " << !omxPlayer.isPaused();
			omxPlayer.setPaused(!omxPlayer.isPaused());
			break;
		}
			
		case '1':
		{
			
			ofLogVerbose() << "decreaseVolume";
			omxPlayer.decreaseVolume();
			break;
		}
		case '2':
		{
			ofLogVerbose() << "increaseVolume";
			omxPlayer.increaseVolume();
			break;
		}
		case 'b':
		{
			ofLogVerbose() << "stepFrameForward";
			omxPlayer.stepFrameForward();
			break;
		}
			
		default:
		{
			break;
		}	
	}
	
}
