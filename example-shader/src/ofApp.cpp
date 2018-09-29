#include "ofApp.h"


void ofApp::onCharacterReceived(KeyListenerEventData& e)
{
	keyPressed((int)e.character);
}

//--------------------------------------------------------------
void ofApp::setup()
{
    doShader = true;
	consoleListener.setup(this);
	ofHideCursor();
		
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);
	
	ofxOMXPlayerSettings settings;
	settings.videoPath = videoPath;
	
	
	shader.load("shaderExample");

    omxPlayer.setup(settings);
	
}



//--------------------------------------------------------------
void ofApp::update()
{
    
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	
    if(doShader)
    {
        shader.begin();
        shader.setUniformTexture("tex0", omxPlayer.getTextureReference(), omxPlayer.getTextureID());
        shader.setUniform1f("time", ofGetElapsedTimef());
        shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
        omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
        shader.end();
    }else
    {
        omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());

    }
    stringstream info;
    info << omxPlayer.getInfo();
    info << "\n";
    info << "\n";
    info << "PRESS p TO TOGGLE PAUSE";
    info << "\n";
    info << "PRESS v TO SEEK TO STEP FORWARD 1 FRAME";
    info << "\n";
    info << "PRESS V TO SEEK TO STEP FORWARD 5 FRAMES";
    info << "\n";
    info << "PRESS - TO SEEK TO DECREASE VOLUME";
    info << "\n";
    info << "PRESS + or = TO SEEK TO INCREASE VOLUME";
    info << "\n";
    info << "PRESS s TO TOGGLE SHADER";
    
    ofDrawBitmapStringHighlight(info.str() , 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
    
	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
	
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
	 

	ofLog(OF_LOG_VERBOSE, "%c keyPressed", key);
	switch (key) 
	{
        case ' ':
		case 'p':
		{
			ofLogVerbose() << "pause: " << !omxPlayer.isPaused();
			omxPlayer.togglePause();
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
            doShader = !doShader;
            break;
        }
		default:
		{
			break;
		}	
	}
	
}
