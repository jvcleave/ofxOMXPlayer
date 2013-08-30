#include "playlistApp.h"

//This app is a demo of the ability to play multiple files with the Non-Texture Player
//It requires multiple video files to be in /home/pi/videos/current
//There is a bit of glitching while the files switch

//This also demonstrates the ofxOMXPlayerListener pattern available

//If your app extends ofxOMXPlayerListener you will receive an event when the video ends

void playlistApp::onVideoEnd(ofxOMXPlayerListenerEventData& e)
{
	ofLogVerbose(__func__) << " RECEIVED";
	
	//omxPlayer->waitForThread(true);
	omxPlayer->Lock();
	delete omxPlayer;
	omxPlayer = NULL;
	if(videoCounter+1<files.size())
	{
		videoCounter++;
	}else
	{
		videoCounter = 0;
	}
	createPlayer();
}


//--------------------------------------------------------------
void playlistApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	omxPlayer = NULL;
	
		
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
	ofxOMXPlayerSettings settings;
	settings.videoPath = files[videoCounter].path();
	settings.useHDMIForAudio = true;	//default true
	settings.enableTexture = false;		//default true
	settings.enableLooping = false;		//default true
	
	settings.listener = this; //this app extends ofxOMXPlayerListener so it will receive events 
	if(!omxPlayer)
	{
		omxPlayer = new ofxOMXPlayer();
	}
	
	omxPlayer->setup(settings);
}

//--------------------------------------------------------------
void playlistApp::update()
{
	if (!omxPlayer) 
	{
		return;
	}
	if(!omxPlayer->isPlaying() || !omxPlayer->isTextureEnabled)
	{
		return;
	}
	
	
}


//--------------------------------------------------------------
void playlistApp::draw(){
	
	if (!omxPlayer) 
	{
		return;
	}
	
	if(!omxPlayer->isPlaying() && !omxPlayer->isTextureEnabled)
	{
		return;
	}
	
	
	omxPlayer->draw(0, 0, ofGetWidth(), ofGetHeight());
		
	//draw a smaller version in the lower right
	int scaledHeight = omxPlayer->getHeight()/4;
	int scaledWidth = omxPlayer->getWidth()/4;
	omxPlayer->draw(ofGetWidth()-scaledWidth, ofGetHeight()-scaledHeight, scaledWidth, scaledHeight);
	

	/*stringstream info;
	info << "APP FPS: "+ ofToString(ofGetFrameRate());
	info <<"\n" <<	"MEDIA TIME: "			<< omxPlayer->getMediaTime();
	info <<"\n" <<	"DIMENSIONS: "			<< omxPlayer->getWidth()<<"x"<<omxPlayer->getHeight();
	info <<"\n" <<	"DURATION: "			<< omxPlayer->getDuration();
	info <<"\n" <<	"TOTAL FRAMES: "		<< omxPlayer->getTotalNumFrames();
	info <<"\n" <<	"CURRENT FRAME: "		<< omxPlayer->getCurrentFrame();
	info <<"\n" <<	"REMAINING FRAMES: "	<< omxPlayer->getTotalNumFrames() - omxPlayer->getCurrentFrame();
	info <<"\n" <<	"CURRENT VOLUME: "		<< omxPlayer->getVolume();
	
	
	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);*/
}

//--------------------------------------------------------------
void playlistApp::keyPressed  (int key){

	
}

