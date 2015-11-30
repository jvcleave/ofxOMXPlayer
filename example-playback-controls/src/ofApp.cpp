#include "ofApp.h"




void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    
	consoleListener.setup(this);
	ofHideCursor();
	
    doPause = false;
    doRestart = false;
    doSeek = false;
    doIncreaseSpeed = false;
    doSetNormalSpeed = false;
    doStep = false;
    
    
    
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);
	
	settings.videoPath = videoPath;
    settings.useHDMIForAudio = true;	//default true
    settings.enableLooping = true;		//default true
    settings.enableTexture = true;		//default true
    omxPlayer.setup(settings);
	
}


void ofApp::update()
{
    if(doPause)
    {
        doPause = false;
        omxPlayer.togglePause();
    }
    
    if(doRestart)
    {
        doRestart = false;
        omxPlayer.restartMovie();
    }
    
    if(doSeek)
    {
        doSeek = false;
        int timeInSecondsToSeekTo = ofRandom(2, omxPlayer.getDurationInSeconds()-5);
        omxPlayer.seekToTimeInSeconds(timeInSecondsToSeekTo);
    }
    if(doIncreaseSpeed)
    {
        doIncreaseSpeed = false;
        omxPlayer.increaseSpeed();
    }
    if (doSetNormalSpeed) 
    {
        doSetNormalSpeed = false;
        omxPlayer.setNormalSpeed();
    }
    
    if(doStep)
    {
        doStep = false;
        omxPlayer.stepFrameForward();
    }
    
}

void ofApp::draw()
{
    
    
    ofBackgroundGradient(ofColor::red, ofColor::black);
    omxPlayer.draw(0, 0);
    
    stringstream info;
    info << omxPlayer.getInfo() << endl;
    info << endl;
    info << "COMMANDS" << endl;
    info << "PRESS p TO PAUSE" << endl;
    info << "PRESS s TO STEP FRAME" << endl;
    info << "PRESS r TO RESTART" << endl;
    info << "PRESS i TO INCREASE SPEED" << endl;
    info << "PRESS n FOR NORMAL SPEED" << endl;
	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
}


void ofApp::keyPressed  (int key){
	 
	ofLog(OF_LOG_VERBOSE, "%c keyPressed", key);
	switch (key) 
	{
		
        case 'p':
        {
            doPause = true;
            break;
        }
        case 'r':
        {
            doRestart = true;
            break;
        }
        case 'i':
        {
            doIncreaseSpeed = true;
            break;
        }
        case 'n':
        {
            doSetNormalSpeed = true;
            break;
        }
        case 's':
        {
            doStep = true;
            break;
        }
		default:
		{
			break;
		}	
	}
	
}
