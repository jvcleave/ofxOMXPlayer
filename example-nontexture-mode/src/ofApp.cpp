#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);
	
	//Somewhat like ofFboSettings we may have a lot of options so this is the current model
	ofxOMXPlayerSettings settings;
	settings.videoPath = videoPath;
	settings.useHDMIForAudio = true;	//default true
	settings.enableTexture = false;		//default true
	settings.enableLooping = true;		//default true
	settings.enableAudio = true;		//default true, save resources by disabling
	//settings.doFlipTexture = true;		//default false
	
	
	//so either pass in the settings
	omxPlayer.setup(settings);
	
	//or live with the defaults
	//omxPlayer.loadMovie(videoPath);
	
}


//--------------------------------------------------------------
void ofApp::update()
{
		
}


//--------------------------------------------------------------
void ofApp::draw(){
    

}


