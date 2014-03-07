#include "multiPlayer.h"

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

void multiPlayer::exit()
{
	//omxPlayer.close();
}

//--------------------------------------------------------------
void multiPlayer::update()
{
	
	
	
}


//--------------------------------------------------------------
void multiPlayer::draw(){
	ofBackgroundGradient(ofColor::red, ofColor::black, OF_GRADIENT_BAR);

	for (int i=0; i<omxPlayers.size(); i++) 
	{
		ofxOMXPlayer* player = omxPlayers[i];
		if (player->isPlaying()) 
		{
			if (player->isTextureEnabled) 
			{
				int scaledWidth = player->getWidth()/2;
				
				player->draw(scaledWidth*i, 0, scaledWidth, player->getHeight()/2);
			}
			
			
			stringstream info;
			info <<"\n" <<	"MEDIA TIME: "			<< player->getMediaTime();
			info <<"\n" <<	"DIMENSIONS: "			<< player->getWidth()<<"x"<<player->getHeight();
			info <<"\n" <<	"DURATION: "			<< player->getDuration();
			info <<"\n" <<	"TOTAL FRAMES: "		<< player->getTotalNumFrames();
			info <<"\n" <<	"CURRENT FRAME: "		<< player->getCurrentFrame();
			info <<"\n" <<	"REMAINING FRAMES: "	<< player->getTotalNumFrames() - player->getCurrentFrame();
			info <<"\n" <<	"CURRENT VOLUME: "		<< player->getVolume();
			
			
			ofDrawBitmapStringHighlight(info.str(), player->settings.displayRect.x, 60, ofColor(ofColor::black, 90), ofColor::yellow);
		}
		
	}
	stringstream fpsInfo;
	fpsInfo <<"\n" <<  "APP FPS: "+ ofToString(ofGetFrameRate());
	ofDrawBitmapStringHighlight(fpsInfo.str(), 60, 20, ofColor(ofColor::white, 90), ofColor::red);
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

//--------------------------------------------------------------
void multiPlayer::keyReleased(int key){

}

//--------------------------------------------------------------
void multiPlayer::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void multiPlayer::mouseDragged(int x, int y, int button){


}

//--------------------------------------------------------------
void multiPlayer::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void multiPlayer::mouseReleased(int x, int y, int button){

}


//--------------------------------------------------------------
void multiPlayer::windowResized(int w, int h){

}

//--------------------------------------------------------------
void multiPlayer::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void multiPlayer::dragEvent(ofDragInfo dragInfo){ 

}

