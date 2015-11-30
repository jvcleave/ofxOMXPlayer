#include "ofApp.h"


void ofApp::onCharacterReceived(KeyListenerEventData& e)
{
	keyPressed((int)e.character);
}


void ofApp::setup()
{
	consoleListener.setup(this);
	ofHideCursor();
		
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);
	
	settings.videoPath = videoPath;
    settings.useHDMIForAudio = true;	//default true
    settings.enableLooping = true;		//default true
    settings.enableTexture = true;		//default true
    omxPlayer.setup(settings);
	
}


void ofApp::update()
{

    
}

void ofApp::draw()
{
    ofBackgroundGradient(ofColor::red, ofColor::black);
    int randomPos = ofRandom(0, ofGetWidth()/2);
    omxPlayer.draw(randomPos,
                   randomPos,
                   omxPlayer.getWidth(),
                   omxPlayer.getHeight() );
	ofDrawBitmapStringHighlight(omxPlayer.getInfo(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
}


void ofApp::keyPressed  (int key){
	 
	ofLog(OF_LOG_VERBOSE, "%c keyPressed", key);
	switch (key) 
	{
		
			
		default:
		{
			break;
		}	
	}
	
}
