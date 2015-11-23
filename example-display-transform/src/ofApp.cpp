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
	
    
    ofDirectory currentVideoDirectory(ofToDataPath("/home/pi/videos/current", true));
    if (currentVideoDirectory.exists())
    {
        currentVideoDirectory.listDir();
        currentVideoDirectory.sort();
        vector<ofFile> files = currentVideoDirectory.getFiles();
        if (files.size()>0)
        {
           videoPath = files[0].path();
        }		
    }
    
	
	settings.videoPath = videoPath;
    settings.useHDMIForAudio = true;	//default true
    settings.enableLooping = true;		//default true
    settings.enableTexture = false;		//default true
    omxPlayer.setup(settings);
	
}


void ofApp::update()
{
    if(ofGetFrameNum() % 3 == 0)
    {
        omxPlayer.setDisplayRectForNonTexture(ofRandom(ofGetScreenWidth()),
                                              ofRandom(ofGetScreenHeight()),
                                              ofRandom(ofGetScreenWidth()),
                                              ofRandom(ofGetScreenHeight()));
    }
    
}

void ofApp::draw()
{
    ofBackgroundGradient(ofColor::red, ofColor::black);
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
