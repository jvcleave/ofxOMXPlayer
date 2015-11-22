#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	
    //be aware bin/data has to exist even though the video is not in the folder.
    
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);
	

    
    rpiVideoPlayer.load(videoPath);
	
}



//--------------------------------------------------------------
void ofApp::update()
{
    rpiVideoPlayer.update();	
}


//--------------------------------------------------------------
void ofApp::draw()
{
    rpiVideoPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
                            
    int scaledHeight	= rpiVideoPlayer.getHeight()/4;
    int scaledWidth		= rpiVideoPlayer.getWidth()/4;
    rpiVideoPlayer.draw(ofGetWidth()-scaledWidth, ofGetHeight()-scaledHeight, scaledWidth, scaledHeight);
                        
    ofDrawBitmapStringHighlight(rpiVideoPlayer.omxPlayer.getInfo(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);

}


