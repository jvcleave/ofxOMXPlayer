#include "playlistApp.h"

//This app is a demo of the ability to play multiple files with the Non-Texture Player
//It requires multiple video files to be in /home/pi/videos/current
//There is a bit of glitching while the files switch

//This also demonstrates the ofxOMXPlayerListener pattern available

//If your app extends ofxOMXPlayerListener you will receive an event when the video ends






bool doLoadNextMovie = false;
void playlistApp::onVideoEnd(ofxOMXPlayerListenerEventData& e)
{
	ofLogVerbose(__func__) << " RECEIVED";
	doLoadNextMovie = true;
}


void playlistApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
	keyPressed((int)e.character);
}



unsigned long long skipTimeStart=0;
unsigned long long skipTimeEnd=0;
unsigned long long amountSkipped =0;
unsigned long long totalAmountSkipped =0;
//--------------------------------------------------------------
void playlistApp::setup()
{
	ofBackground(ofColor::black);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(false);
	consoleListener.setup(this);	
	//this will let us just grab a video without recompiling
	ofDirectory currentVideoDirectory("/home/pi/videos/current");
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
			settings.enableLooping = false;
			//settings.enableTexture = false;		//default true
			
			
			//settings.enableAudio = !settings.enableAudio; //toggle for testing
			settings.listener = this; //this app extends ofxOMXPlayerListener so it will receive events ;
			omxPlayer.setup(settings);
		}		
	}
}


void playlistApp::loadNextMovie()
{
	if(videoCounter+1<files.size())
	{
		videoCounter++;
	}else
	{
		videoCounter = 0;
	}
	skipTimeStart = ofGetElapsedTimeMillis();
	omxPlayer.loadMovie(files[videoCounter].path());
	skipTimeEnd = ofGetElapsedTimeMillis();
	amountSkipped = skipTimeEnd-skipTimeStart;
	totalAmountSkipped+=amountSkipped;
	doLoadNextMovie = false;
}

//--------------------------------------------------------------
void playlistApp::update()
{
	if (doLoadNextMovie) 
	{
		ofLogVerbose(__func__) << "doing reload";
		//with the texture based player this must be done here - especially if the videos are different resolutions
		
		loadNextMovie();
		
	}
		
	
}


//--------------------------------------------------------------
void playlistApp::draw(){
	
	
	
	if(!omxPlayer.isTextureEnabled)
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
	if (omxPlayer.getTotalNumFrames() >0) 
	{
		info <<"\n" <<	"REMAINING FRAMES: "	<< omxPlayer.getTotalNumFrames() - omxPlayer.getCurrentFrame();
	}else 
	{
		info <<"\n" <<	"FILE IS LIKELY H264 STREAM";
	}

	
	info <<"\n" <<	"CURRENT VOLUME: "		<< omxPlayer.getVolume();
	
	
	
	info <<"\n" <<	"MILLIS SKIPPED: "		<< amountSkipped;
	info <<"\n" <<	"TOTAL MILLIS SKIPPED: " << totalAmountSkipped;
	info <<"\n" <<	"CURRENT MOVIE: "		<< files[videoCounter].path();
	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
}

//--------------------------------------------------------------
void playlistApp::keyPressed  (int key){

	ofLogVerbose(__func__) << "key: " << key;
	switch (key) 
	{
		case 'n':
		{
			doLoadNextMovie = true;
			break;
		}
		case 'e':
		{
			break;
		}
		case 'x':
		{
			break;
		}
	}
}

