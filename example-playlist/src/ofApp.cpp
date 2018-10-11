#include "ofApp.h"

//This app is a demo of the ability to play multiple files with the Non-Texture Player
//It requires multiple video files to be in /home/pi/videos/current
//There is a bit of glitching while the files switch

//This also demonstrates the ofxOMXPlayerListener pattern available

//If your app extends ofxOMXPlayerListener you will receive an event when the video ends or loops





bool doLoadNextMovie = false;


void ofApp::onVideoEnd(ofxOMXPlayer* player)
{
    ofLog() << "onVideoEnd: " << player->isLoopingEnabled();
    doLoadNextMovie = true;

}

void ofApp::onVideoLoop(ofxOMXPlayer* player)
{
    
}


void ofApp::onCharacterReceived(KeyListenerEventData& e)
{
	keyPressed((int)e.character);
}



unsigned long long skipTimeStart=0;
unsigned long long skipTimeEnd=0;
unsigned long long amountSkipped =0;
unsigned long long totalAmountSkipped =0;
//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    
	ofBackground(ofColor::black);
	consoleListener.setup(this);	
	
	//this will let us just grab a video without recompiling
    string directoryPath = ofToDataPath("../../../video", true);
	ofDirectory currentVideoDirectory(directoryPath);
	if (currentVideoDirectory.exists()) 
	{
		currentVideoDirectory.listDir();
		currentVideoDirectory.sort();
		files = currentVideoDirectory.getFiles();
		if (files.size()>0) 
		{
			videoCounter = 0;
			settings.videoPath = files[videoCounter].path();
			settings.useHDMIForAudio = true;	//default true
			settings.enableLooping = false;		//default true
			settings.enableTexture = true;		//default true
			settings.listener = this;			//this app extends ofxOMXPlayerListener so it will receive events ;
			omxPlayer.setup(settings);
		}		
	}else
    {
        ofLog() << directoryPath << "DOES NOT EXIST - does bin/data exist?";
    }
}


void ofApp::loadNextMovie()
{
	if(videoCounter+1<files.size())
	{
		videoCounter++;
	}else
	{
		videoCounter = 0;
	}
	skipTimeStart = ofGetElapsedTimeMillis();
    ofLog() << "LOADING MOVIE" << files[videoCounter].path();
	omxPlayer.loadMovie(files[videoCounter].path());
	skipTimeEnd = ofGetElapsedTimeMillis();
	amountSkipped = skipTimeEnd-skipTimeStart;
	totalAmountSkipped+=amountSkipped;
	doLoadNextMovie = false;
}

//--------------------------------------------------------------
void ofApp::update()
{
	if (doLoadNextMovie) 
	{
		ofLogVerbose(__func__) << "doing reload";
		
		if(omxPlayer.isTextureEnabled())
		{
			//clear the texture if you want
			//omxPlayer.getTextureReference().clear();
		}
		//with the texture based player this must be done here - especially if the videos are different resolutions
		loadNextMovie();
	}
		
	
}


//--------------------------------------------------------------
void ofApp::draw(){
	
	//ofBackgroundGradient(ofColor::red, ofColor::black, OF_GRADIENT_CIRCULAR);
	
	if(!omxPlayer.isTextureEnabled()) return;
	
	omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
	
	//draw a smaller version in the lower right
	int scaledHeight = omxPlayer.getHeight()/4;
	int scaledWidth = omxPlayer.getWidth()/4;
	omxPlayer.draw(ofGetWidth()-scaledWidth, ofGetHeight()-scaledHeight, scaledWidth, scaledHeight);

	stringstream info;
    info << omxPlayer.getInfo() << endl;
    info << "MILLIS SKIPPED: " << amountSkipped << endl;
	info << "TOTAL MILLIS SKIPPED: " << totalAmountSkipped << endl;
	info << "CURRENT MOVIE: " << files[videoCounter].path() << endl;
    info << "PRESS n TO LOAD NEXT MOVIE"<< endl;
	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){

	ofLogVerbose(__func__) << "key: " << key;
	switch (key) 
	{
		case 'n':
		{
			doLoadNextMovie = true;
			break;
		}
	}
}


