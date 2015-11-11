#include "ofApp.h"


void ofApp::onCharacterReceived(KeyListenerEventData& e)
{
	keyPressed((int)e.character);
}


void ofApp::onVideoEnd(ofxOMXPlayerListenerEventData& e)
{
	ofLogVerbose(__func__) << "at: " << ofGetElapsedTimeMillis();
}
void ofApp::onVideoLoop(ofxOMXPlayerListenerEventData& e)
{
	ofLogVerbose(__func__) << "at: " << ofGetElapsedTimeMillis();
}


//--------------------------------------------------------------
void ofApp::setup()
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
void ofApp::update()
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
void ofApp::draw(){
	
	fbo.draw(0, 0);
    
    stringstream info;
    info << omxPlayer.getInfo();
    info << "\n";
    info << "\n";
    info << "PRESS p TO TOGGLE PAUSE";
    info << "\n";
    info << "PRESS b TO SEEK TO FRAME STEP FORWARD";
    info << "\n";
    info << "PRESS 1 TO SEEK TO DECREASE VOLUME";
    info << "\n";
    info << "PRESS 2 TO SEEK TO INCREASE VOLUME";
    ofDrawBitmapStringHighlight(info.str() , 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
    
	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
	
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
	 
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
