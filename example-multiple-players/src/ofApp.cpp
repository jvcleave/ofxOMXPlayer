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
			settings.enableTexture = i==1;		//default true
            
			
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
        float halfWidth = player->getWidth()*.5;
        
        ofRectangle drawRect(halfWidth*i,
                             0,
                             halfWidth,
                             player->getHeight()*.5);

        player->draw(drawRect); 
		
        ofDrawBitmapStringHighlight(player->getInfo(), drawRect.x, drawRect.getHeight()+20, ofColor(ofColor::black, 90), ofColor::yellow);

	}

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

