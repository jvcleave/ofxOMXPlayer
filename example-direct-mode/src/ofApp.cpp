#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    
    
    //terminalListener.setup(this);
    
    omxPlayer = NULL;
    
	
	
	//or live with the defaults
	//omxPlayer.loadMovie(videoPath);
	
}

void ofApp::createPlayer()
{
    if(!omxPlayer)
    {
        ofLog() << "createPlayer";
        
        string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);
        
        //Somewhat like ofFboSettings we may have a lot of options so this is the current model
        ofxOMXPlayerSettings settings;
        settings.videoPath = videoPath;
        settings.useHDMIForAudio = true;    //default true
        settings.enableTexture = false;        //default true
        settings.enableLooping = true;        //default true
        settings.enableAudio = true;        //default true, save resources by disabling
        //settings.doFlipTexture = true;        //default false
        
        omxPlayer = new ofxOMXPlayer();
        //so either pass in the settings
        omxPlayer->setup(settings);
    }

}

//--------------------------------------------------------------
void ofApp::update()
{
		
}


//--------------------------------------------------------------
void ofApp::draw(){
    if (ofGetFrameNum() == 100)
    {
        createPlayer();
    }

}

void ofApp::onCharacterReceived(KeyListenerEventData& e)
{
    keyPressed(e.character);

}

void ofApp::keyPressed  (int key){
    
    ofLog(OF_LOG_VERBOSE, "%c keyPressed", key);
    if (omxPlayer)
    {
        switch (key)
        {
            case ' ':
            case 'p':
            {
                omxPlayer->togglePause();
                break;
            }
            case '1':
            {
                omxPlayer->decreaseSpeed();
                break;
            }
            case '2':
            {
                omxPlayer->increaseSpeed();
                break;
            }
            case '3':
            {
                omxPlayer->setNormalSpeed();
                break;
            }
            case '-':
            {
                
                ofLogVerbose() << "decreaseVolume";
                omxPlayer->decreaseVolume();
                break;
            }
            case '=':
            case '+':
            {
                ofLogVerbose() << "increaseVolume";
                omxPlayer->increaseVolume();
                break;
            }
            case 'v':
            {
                ofLogVerbose() << "stepFrameForward";
                omxPlayer->stepFrameForward();
                break;
            }
            case 'V':
            {
                ofLogVerbose() << "stepNumFrames 5";
                omxPlayer->stepNumFrames(5);
                break;
            }
            case 's':
            {
                //doSeek = true;
                break;
            }
            case 'r':
            {
                omxPlayer->restartMovie();
                break;
            }
            case '>':
            case '.':
            {
                omxPlayer->increaseSpeed();
                break;
            }
            case '<':
            case ',':
            {
                omxPlayer->decreaseSpeed();
                break;
            }
            case 'q':
            {
                omxPlayer->close();
                break;
            }
            case 'o':
            {
                //doReopen = true;
                break;
            }
            case 'n':
            {
                //doLoadNext = true;
                break;
            }
                
            default:
            {
                break;
            }
        }
    }else
    {
        switch (key)
        {
            case ' ':
            {
                if(!omxPlayer)
                {
                    createPlayer(); 
                }
                break;
            }
        }
    }
}
