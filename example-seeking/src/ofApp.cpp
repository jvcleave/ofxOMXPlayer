#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup()
{
    doSeek = false;
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
		
	//Somewhat like ofFboSettings we may have a lot of options so this is the current model
	ofxOMXPlayerSettings settings;
	settings.videoPath = videoPath;
	settings.useHDMIForAudio	= true;		//default true
	settings.enableTexture		= true;		//default true
	settings.enableLooping		= true;		//default true
	settings.enableAudio		= true;		//default true, save resources by disabling
	//settings.doFlipTexture = true;		//default false
	
	if (!settings.enableTexture) 
	{
		/*
		 We have the option to pass in a rectangle
		 to be used for a non-textured player to use (as opposed to the default full screen)
		 */
		settings.displayRect.width = 400;
		settings.displayRect.height = 300;
		settings.displayRect.x = 440;
		settings.displayRect.y = 200;
	}
	
	
	//so either pass in the settings
	omxPlayer.setup(settings);
	
	consoleListener.setup(this);

    
    
}


//--------------------------------------------------------------
void ofApp::update()
{
	if(doSeek)
    {
        int timeInSecondsToSeekTo = ofRandom(2, omxPlayer.getDurationInSeconds()-5);
        omxPlayer.seekToTimeInSeconds(timeInSecondsToSeekTo);
        doSeek = false;
       
    }
	
}


//--------------------------------------------------------------
void ofApp::draw()
{
	if (omxPlayer.isTextureEnabled())
    {
        omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
    }
	
    stringstream info;
    info << omxPlayer.getInfo();
    info << "\n";
    info << "\n";
    info << "PRESS p TO TOGGLE PAUSE";
    info << "\n";
    info << "PRESS s TO SEEK TO RANDOM TIME";
	ofDrawBitmapStringHighlight(info.str() , 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
  
    
}

void ofApp::keyPressed(int key)
{
    ofLog(OF_LOG_VERBOSE, "%c keyPressed", key);
	if (key == 'p') 
	{
		omxPlayer.setPaused(!omxPlayer.isPaused());
	}
	if (key == 's')
	{
        doSeek = true;
	}
}

void ofApp::onCharacterReceived(KeyListenerEventData& e)
{
	keyPressed((int)e.character);
}
