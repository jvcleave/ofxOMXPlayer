#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{	
	ofDirectory currentVideoDirectory(ofToDataPath("../../../video", true));
	if (currentVideoDirectory.exists()) 
	{
		currentVideoDirectory.listDir();
		vector<ofFile> files = currentVideoDirectory.getFiles();
		
		
		for (int i=0; i<files.size(); i++) 
		{
			ofxOMXPlayerSettings settings;
			settings.videoPath = files[i].path();
			settings.useHDMIForAudio = true;	//default true
			settings.enableLooping = true;		//default true
			settings.enableAudio = true;		//default true, save resources by disabling
			settings.enableTexture = true;		//default true
            
            settings.drawRectangle.x = 40+(400*i);
            settings.drawRectangle.y = 100;
            
            settings.drawRectangle.width = 400;
            settings.drawRectangle.height = 300;
			
			ofxOMXPlayer* player = new ofxOMXPlayer();
            
			player->setup(settings);
            omxPlayers.emplace_back(player);
		}
    }else{
        ofLogError() << "currentVideoDirectory: " << currentVideoDirectory.path() << " MISSING";
        
        
    }

}

//--------------------------------------------------------------
void ofApp::update()
{
	
	
	
}


//--------------------------------------------------------------
void ofApp::draw(){
	ofBackgroundGradient(ofColor::red, ofColor::black, OF_GRADIENT_BAR);
	for (int i=0; i<omxPlayers.size(); i++) 
	{
		ofxOMXPlayer* player = omxPlayers[i];
        player->draw(player->settings.drawRectangle.x,
                     player->settings.drawRectangle.y,
                     player->settings.drawRectangle.getWidth(),
                     player->settings.drawRectangle.getHeight());
        ofDrawBitmapStringHighlight(player->getInfo(), player->settings.drawRectangle.x, 60, ofColor(ofColor::black, 90), ofColor::yellow);

	}
	stringstream fpsInfo;
	fpsInfo <<"\n" <<  "APP FPS: "+ ofToString(ofGetFrameRate());
	ofDrawBitmapStringHighlight(fpsInfo.str(), 60, 20, ofColor::black, ofColor::yellow);
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
	switch (key) 
	{
		case 'c':
		{
			break;
		}
	}
	
}

