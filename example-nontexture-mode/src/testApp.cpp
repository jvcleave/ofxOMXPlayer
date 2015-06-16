#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup()
{
    //ofSetLogLevel("ofThread", OF_LOG_SILENT);
    ofSetLogLevel(OF_LOG_VERBOSE);
	
    
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

bool forceClosed = false;

//--------------------------------------------------------------
void testApp::update()
{
		if(ofGetElapsedTimeMillis()>10000)
        {
            if(!forceClosed)
            {
                omxPlayer.close();
                ofLogVerbose(__func__)  << "";

                forceClosed = true;

            }
            
        }
}


//--------------------------------------------------------------
void testApp::draw(){
    
    return;
	if(!omxPlayer.isTextureEnabled())
	{
		return;
	}
	
	omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
	
	//draw a smaller version in the lower right
	int scaledHeight	= omxPlayer.getHeight()/4;
	int scaledWidth		= omxPlayer.getWidth()/4;
	omxPlayer.draw(ofGetWidth()-scaledWidth, ofGetHeight()-scaledHeight, scaledWidth, scaledHeight);

	ofDrawBitmapStringHighlight(omxPlayer.getInfo(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
}


