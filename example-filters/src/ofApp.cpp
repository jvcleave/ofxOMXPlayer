#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
    consoleListener.setup(this);
    doChangeFilter = false;
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);
	
	//Somewhat like ofFboSettings we may have a lot of options so this is the current model
	ofxOMXPlayerSettings settings;
	settings.videoPath = videoPath;
	settings.useHDMIForAudio = true;	//default true
	settings.enableTexture = true;		//default true
	settings.enableLooping = true;		//default true
	settings.enableAudio = true;		//default true, save resources by disabling
	//settings.doFlipTexture = true;		//default false
	settings.enableFilters = true;
    settings.filter = OMX_ImageFilterPosterise;
	//so either pass in the settings
	omxPlayer.setup(settings);
	
	//or live with the defaults
	//omxPlayer.loadMovie(videoPath);
    filterCollection.setup();
}



//--------------------------------------------------------------
void ofApp::update()
{
    if(doChangeFilter)
    {
        doChangeFilter = false;
        OMX_IMAGEFILTERTYPE filter = filterCollection.getNextFilter();
        omxPlayer.applyFilter(filter);
    }
}


//--------------------------------------------------------------
void ofApp::draw(){
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


void ofApp::keyPressed  (int key){
    
    ofLog(OF_LOG_VERBOSE, "%c keyPressed", key);
    switch (key) 
    {
            
        case 'f':
        {
            doChangeFilter = true;
            break;
        }
        default:
        {
            break;
        }	
    }
    
}

