#include "ofApp.h"


bool doPixels = false;

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	
    //be aware bin/data has to exist even though the video is not in the folder.
    
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);
	

    bool doPixels = true; //slow
    if(doPixels)
    {
       rpiVideoPlayer.enablePixels(); 
    }
    
    rpiVideoPlayer.load(videoPath);
    
}



//--------------------------------------------------------------
void ofApp::update()
{
    rpiVideoPlayer.update();
    
    if(rpiVideoPlayer.pixelsEnabled())
    {
        if (!pixelOutput.isAllocated()) 
        {
            pixelOutput.allocate(rpiVideoPlayer.getWidth(), rpiVideoPlayer.getHeight(), GL_RGBA);
        } 
        pixelOutput.loadData(rpiVideoPlayer.getPixels().getData(), rpiVideoPlayer.getWidth(), rpiVideoPlayer.getHeight(), GL_RGBA);
    }
    
    
    
}


//--------------------------------------------------------------
void ofApp::draw()
{
    rpiVideoPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
                            
    int scaledHeight	= rpiVideoPlayer.getHeight()/4;
    int scaledWidth		= rpiVideoPlayer.getWidth()/4;
    rpiVideoPlayer.draw(ofGetWidth()-scaledWidth, ofGetHeight()-scaledHeight, scaledWidth, scaledHeight);
    
    if(rpiVideoPlayer.pixelsEnabled())
    {
        pixelOutput.draw(20, 20, rpiVideoPlayer.getWidth()/2, rpiVideoPlayer.getHeight()/2);
    }
                        
    ofDrawBitmapStringHighlight(rpiVideoPlayer.omxPlayer.getInfo(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);

}


