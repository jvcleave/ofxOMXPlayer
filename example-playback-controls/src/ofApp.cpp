#include "ofApp.h"



vector<int>seekTargets;
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
    doFrameStep = false;
    
    
    
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);
	
	settings.videoPath = videoPath;
    settings.useHDMIForAudio = true;	//default true
    settings.enableLooping = true;		//default true
//    settings.enableTexture = false;		//default true
    omxPlayer.setup(settings);
    
    
	
}

int currentTargetIndex = 0;
void ofApp::update()
{
   
    if(seekTargets.empty())
    {
        int numFrames = omxPlayer.getTotalNumFrames();
        int numTargets = 8;
        for(int i=1; i<numTargets-1; i++)
        {
            int target = numFrames/i;
            ofLog() << "adding target: " << target;
            seekTargets.push_back(target);
            
        }
    }
    if(doPause)
    {
       if(!omxPlayer.isPaused())
       {
           omxPlayer.setPaused(true);
       }
        doPause = false;
    }
    if(doRestart)
    {
        doRestart = false;
        //omxPlayer.restartMovie();
    }
    
    if(doSeek)
    {
        doSeek = false;
        int frameTarget = seekTargets[currentTargetIndex];
        if(currentTargetIndex+1<seekTargets.size())
        {
            currentTargetIndex++;
        }else
        {
            currentTargetIndex = 0;
        }
        omxPlayer.seekToFrame(frameTarget);
    }

    
}

void ofApp::draw()
{
    
    
    ofBackgroundGradient(ofColor::red, ofColor::black);
    omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
    
    stringstream info;
    info << omxPlayer.getInfo() << endl;
    info << endl;
    info << "COMMANDS" << endl;
    info << "PRESS p TO PAUSE" << endl;
    info << "PRESS v TO STEP FRAME" << endl;
    //info << "PRESS r TO RESTART" << endl;
    info << "PRESS 1 TO DECREASE SPEED" << endl;
    info << "PRESS 2 TO INCREASE SPEED" << endl;
    info << "PRESS 3 FOR NORMAL SPEED" << endl;
    info << "PRESS s FOR RANDOM SEEK" << endl;
    info << "PRESS r TO RESTART MOVIE" << endl;
    info << "PRESS p TO TOGGLE PAUSE"<< endl;
    info << "PRESS v TO SEEK TO STEP FORWARD 1 FRAME" << endl;
    info << "PRESS V TO SEEK TO STEP FORWARD 5 FRAMES" << endl;
    info << "PRESS - TO SEEK TO DECREASE VOLUME" << endl;
    info << "PRESS + or = TO SEEK TO INCREASE VOLUME" << endl;

	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
}


void ofApp::keyPressed  (int key){
	 
	ofLog(OF_LOG_VERBOSE, "%c keyPressed", key);
	switch (key) 
	{
		case ' ':
        case 'p':
        {
            omxPlayer.togglePause();
            break;
        }
        case '1':
        {
            omxPlayer.decreaseSpeed();
            break;
        }
        case '2':
        {
            omxPlayer.increaseSpeed();
            break;
        }
        case '3':
        {
            omxPlayer.setNormalSpeed();
            break;
        }
        case '-':
        {
            
            ofLogVerbose() << "decreaseVolume";
            omxPlayer.decreaseVolume();
            break;
        }
        case '=':
        case '+':
        {
            ofLogVerbose() << "increaseVolume";
            omxPlayer.increaseVolume();
            break;
        }
        case 'v':
        {
            ofLogVerbose() << "stepFrameForward";
            omxPlayer.stepFrameForward();
            break;
        }
        case 'V':
        {
            ofLogVerbose() << "stepNumFrames 5";
            omxPlayer.stepNumFrames(5);
            break;
        }
        case 's':
        {
            doSeek = true;
            break;
        }
        case 'r':
        {
            omxPlayer.restartMovie();
            break;
        }
        case '>':
        case '.':
        {
            omxPlayer.increaseSpeed();
            break;
        }    
        case '<':
        case ',':
        {
            omxPlayer.decreaseSpeed();
            break;
        }     
        case 'q':
        {
            omxPlayer.close();
            break;
        }
        case 'o':
        {
            omxPlayer.reopen();
            break;
        }
		default:
		{
			break;
		}	
	}
	
}
