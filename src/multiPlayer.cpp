#include "multiPlayer.h"

#if 0
//--------------------------------------------------------------
void multiPlayer::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(false);
	
	//this will let us just grab a video without recompiling
	ofDirectory currentVideoDirectory("/home/pi/videos/current");
	if (currentVideoDirectory.exists()) 
	{
		currentVideoDirectory.listDir();
		vector<ofFile> files = currentVideoDirectory.getFiles();
		
		
		for (int i=0; i<files.size(); i++) 
		{
			//Somewhat like ofFboSettings we may have a lot of options so this is the current model
			ofxOMXPlayerSettings settings;
			settings.videoPath = files[i].path();
			settings.useHDMIForAudio = true;	//default true
			//settings.enableTexture = true;		//default true
			settings.enableLooping = true;		//default true
			settings.enableAudio = true;		//default true, save resources by disabling
			settings.enableTexture = false;		//default true
			if (!settings.enableTexture) 
			{
				settings.displayRect.width = 400;
				settings.displayRect.height = 300;
				settings.displayRect.x = 40+(400*i);
				settings.displayRect.y = 200;
			}
			ofxOMXPlayer* player = new ofxOMXPlayer();
			
			player->setup(settings);
			omxPlayers.push_back(player);
		}
	}

}

#endif

vector<ofRectangle> rects;
//--------------------------------------------------------------
void multiPlayer::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(false);
	ofHideCursor();
	int numCols = 3;
	ofDirectory currentVideoDirectory("/home/pi/videos/current");
	
	string videoPath = "/home/pi/videos/current/AirBallonTimecode.mov";
	if (currentVideoDirectory.exists()) 
	{
		currentVideoDirectory.listDir();
		vector<ofFile> files = currentVideoDirectory.getFiles();
		videoPath = files[0].path();
	}
		
	for (int i=0; i<3; i++) 
	{
		//Somewhat like ofFboSettings we may have a lot of options so this is the current model
		ofxOMXPlayerSettings settings;
		settings.videoPath = videoPath;
		settings.useHDMIForAudio = true;	//default true
		settings.enableTexture = true;		//default true
		settings.enableLooping = true;		//default true
		//settings.enableAudio = true;		//default true, save resources by disabling
		//settings.enableAudio = false;
		//settings.enableTexture = false;		//default true
		
		if(i>0)
		 {
			settings.enableAudio = false;
		 }
		settings.enableAudio = false;
	
		//rects.push_back(settings.displayRect);
		
		int width	= 320;
		int height	= 180;
		
		ofRectangle rect;
		rect.width		= width;
		rect.height		= height;
		rect.x			= width * (i % numCols);
		rect.y			= height * int(i / numCols);
		
		rect.x+=40;
		rect.y+=40;
		rects.push_back(rect);
		
		ofxOMXPlayer* player = new ofxOMXPlayer();
		
		player->setup(settings);
		omxPlayers.push_back(player);
	}
	
	
	
	
	
}

//--------------------------------------------------------------
void multiPlayer::update()
{
	
	
	
}


//--------------------------------------------------------------
void multiPlayer::draw(){
	//ofBackgroundGradient(ofColor::red, ofColor::black, OF_GRADIENT_BAR);
	int numPlayers = omxPlayers.size();
	int width = ofGetWidth();
	int height = ofGetHeight();
	for (int i=0; i<numPlayers; i++) 
	{
		ofxOMXPlayer* player = omxPlayers[i];
		if (player->isPlaying()) 
		{
			ofPushMatrix();
			ofTranslate((width/numPlayers)*i, 200);
			ofTranslate(20, 0);

			player->draw(0, 0, (width/numPlayers), (height/numPlayers));
			ofPopMatrix();
			
			
			/*stringstream info;
			info <<"\n" <<	"MEDIA TIME: "			<< player->getMediaTime();
			info <<"\n" <<	"DIMENSIONS: "			<< player->getWidth()<<"x"<<player->getHeight();
			info <<"\n" <<	"DURATION: "			<< player->getDuration();
			info <<"\n" <<	"TOTAL FRAMES: "		<< player->getTotalNumFrames();
			info <<"\n" <<	"CURRENT FRAME: "		<< player->getCurrentFrame();
			info <<"\n" <<	"REMAINING FRAMES: "	<< player->getTotalNumFrames() - player->getCurrentFrame();
			info <<"\n" <<	"CURRENT VOLUME: "		<< player->getVolume();
			
			
			ofDrawBitmapStringHighlight(info.str(), player->settings.displayRect.x, 60, ofColor(ofColor::black, 90), ofColor::yellow);*/
		}
		
	}
	stringstream fpsInfo;
	fpsInfo <<"\n" <<  "APP FPS: "+ ofToString(ofGetFrameRate());
	ofDrawBitmapStringHighlight(fpsInfo.str(), 60, 20, ofColor::black, ofColor::yellow);
}

//--------------------------------------------------------------
void multiPlayer::keyPressed  (int key){
	switch (key) 
	{
		case 'c':
		{
			break;
		}
	}
	
}

