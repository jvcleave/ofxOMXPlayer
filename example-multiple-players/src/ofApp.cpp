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
			settings.enableTexture = false;		//default true
			if (!settings.enableTexture) 
			{
				/*
				 We have the option to pass in a rectangle
				 to be used for a non-textured player to use (as opposed to the default full screen)
				 */
                
                settings.directDisplayOptions.drawRectangle.x = 40+(400*i);
                settings.directDisplayOptions.drawRectangle.y = 200;
                
				settings.directDisplayOptions.drawRectangle.width = 400;
				settings.directDisplayOptions.drawRectangle.height = 300;
				
				
			}
			
			ofxOMXPlayer* player = new ofxOMXPlayer();
			player->setup(settings);
			omxPlayers.push_back(player);
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
		if (player->isPlaying()) 
		{
			ofPushMatrix();
				ofTranslate(player->drawRectangle->x, 0, 0);
				ofDrawBitmapStringHighlight(player->getInfo(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
			ofPopMatrix();
		}		
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

