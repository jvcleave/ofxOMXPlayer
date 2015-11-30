#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetLogLevel(OF_LOG_NOTICE);
	ofSetLogLevel("ofThread", OF_LOG_ERROR);
		
	
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);

	//videoPath = ofxOMXPlayer::getRandomVideo("/home/pi/videos/current");

		
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
        
        ofRectangle drawRectangle;
		drawRectangle.width = 400;
		drawRectangle.height = 300;
		drawRectangle.x = 440;
		drawRectangle.y = 200;
        
        settings.directDisplayOptions.drawRectangle = drawRectangle;
	}
	
	
	//so either pass in the settings
	omxPlayer.setup(settings);
 
    
	consoleListener.setup(this);
}

//--------------------------------------------------------------
void ofApp::update()
{
    
}


//--------------------------------------------------------------
void ofApp::draw()
{
	if(omxPlayer.isTextureEnabled())
    {
        omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
    }
	
    stringstream info;
    info << omxPlayer.getInfo();
    info << "\n";
    info << "\n";
    info << "speedMultiplier " << omxPlayer.getSpeedMultiplier();
    info << "\n";
    info << "PRESS n to reset Speed";
    info << "\n";
    info << "PRESS i to increase Speed";
    info << "\n";
	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
  
    
}

void ofApp::keyPressed(int key)
{
    ofLog(OF_LOG_VERBOSE, "%c keyPressed", key);
	if (key == 'i')
	{
        /*
            BETA FEATURE:
            If the movie loops it will take a while for the video to recover to normal speed
         
         */
        omxPlayer.increaseSpeed();
	}
	if (key == 'n')
	{
        omxPlayer.setNormalSpeed();
        
	}
}

void ofApp::onCharacterReceived(KeyListenerEventData& e)
{
	keyPressed((int)e.character);
}
