#include "playlistApp.h"

//This app is a demo of the ability to play multiple files with the Non-Texture Player
//It requires multiple video files to be in /home/pi/videos/current
//There is a bit of glitching while the files switch

//This also demonstrates the ofxOMXPlayerListener pattern available

//If your app extends ofxOMXPlayerListener you will receive an event when the video ends





void playlistApp::onVideoEnd(ofxOMXPlayerListenerEventData& e)
{
	ofLogVerbose(__func__) << " RECEIVED";
		
	if(videoCounter+1<files.size())
	{
		videoCounter++;
	}else
	{
		videoCounter = 0;
	}
	omxPlayer.loadMovie(files[videoCounter].path());
	//settings.enableTexture = !settings.enableTexture;
	//createPlayer();
	
}


void playlistApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
	keyPressed((int)e.character);
}


//--------------------------------------------------------------
void playlistApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(false);
	consoleListener.setup(this);	
	//this will let us just grab a video without recompiling
	ofDirectory currentVideoDirectory("/home/pi/videos/current");
	if (currentVideoDirectory.exists()) 
	{
		currentVideoDirectory.listDir();
		files = currentVideoDirectory.getFiles();
		if (files.size()>0) 
		{
			videoCounter = 0;
			createPlayer();
		}		
	}
		
	
	
}
void playlistApp::createPlayer()
{
	
	settings.videoPath = files[videoCounter].path();
	settings.useHDMIForAudio = true;	//default true
	
	settings.enableTexture = false;		//default true
	if(files.size() > 1)
	{
		settings.enableLooping = false;		//default true
	}
	
	
	
	//settings.enableAudio = !settings.enableAudio; //toggle for testing
	settings.listener = this; //this app extends ofxOMXPlayerListener so it will receive events ;
	omxPlayer.setup(settings);
	
}

//--------------------------------------------------------------
void playlistApp::update()
{
	
	if(!omxPlayer.isPlaying() || !omxPlayer.isTextureEnabled)
	{
		return;
	}
	
	
}

unsigned long long skipTimeStart=0;
unsigned long long skipTimeEnd=0;
unsigned long long amountSkipped =0;
bool doingSkipCheck = false;
//--------------------------------------------------------------
void playlistApp::draw(){
	
	
	
	if(!omxPlayer.isPlaying() && !omxPlayer.isTextureEnabled)
	{
		return;
	}
	
	
	omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
		
	//draw a smaller version in the lower right
	int scaledHeight = omxPlayer.getHeight()/4;
	int scaledWidth = omxPlayer.getWidth()/4;
	omxPlayer.draw(ofGetWidth()-scaledWidth, ofGetHeight()-scaledHeight, scaledWidth, scaledHeight);
	

	stringstream info;
	info <<"\n" <<  "APP FPS: "+ ofToString(ofGetFrameRate());
	info <<"\n" <<	"MEDIA TIME: "			<< omxPlayer.getMediaTime();
	info <<"\n" <<	"DIMENSIONS: "			<< omxPlayer.getWidth()<<"x"<<omxPlayer.getHeight();
	info <<"\n" <<	"DURATION: "			<< omxPlayer.getDuration();
	info <<"\n" <<	"TOTAL FRAMES: "		<< omxPlayer.getTotalNumFrames();
	info <<"\n" <<	"CURRENT FRAME: "		<< omxPlayer.getCurrentFrame();
	info <<"\n" <<	"REMAINING FRAMES: "	<< omxPlayer.getTotalNumFrames() - omxPlayer.getCurrentFrame();
	info <<"\n" <<	"CURRENT VOLUME: "		<< omxPlayer.getVolume();
	
	ofColor textColor = ofColor::yellow;
	if(omxPlayer.getCurrentFrame() == 0)
	{
		textColor = ofColor::white;
		if(!doingSkipCheck)
		{
			doingSkipCheck = true;
			skipTimeStart = ofGetElapsedTimeMillis();
		}
	}
	if(doingSkipCheck && textColor == ofColor::yellow)
	{
		skipTimeEnd = ofGetElapsedTimeMillis();
		amountSkipped = skipTimeEnd-skipTimeStart;
		doingSkipCheck = false;
		
	}
	
	info <<"\n" <<	"MILLIS SKIPPED: "		<< amountSkipped;
	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(ofColor::black, 90), textColor);
}

//--------------------------------------------------------------
void playlistApp::keyPressed  (int key){

	ofLogVerbose(__func__) << "key: " << key;
	switch (key) 
	{
		case 'c':
		{
			break;
		}
		case 'e':
		{
			break;
		}
	}
}

