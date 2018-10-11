#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofColor bgColor(ofColor::black, 0);
    
    ofBackground(bgColor);
    ofDirectory videoDirectory(ofToDataPath("../../../video", true));
    if (videoDirectory.exists()) 
    {
        videoDirectory.listDir();
        vector<ofFile> files = videoDirectory.getFiles();
        
        
        for (int i=0; i<files.size(); i++) 
        {
            ofxOMXPlayerSettings settings;
            settings.videoPath = files[i].path();
            settings.enableTexture = false;        //default true            
            ofxOMXPlayer* player = new ofxOMXPlayer();
            if( i == 1)
            {
                settings.directDrawRectangle.set(0, 360*i, 640, 360);
            }
            player->setup(settings); 
            
            
            omxPlayers.push_back(player);
            
        }
    }else
    {
        ofLogError() << "videoDirectory: " << videoDirectory.path() << " MISSING";
        
    }

}


//--------------------------------------------------------------
void ofApp::update()
{
		
}


//--------------------------------------------------------------
void ofApp::draw(){
    
    
    for (int i=0; i<omxPlayers.size(); i++) 
    {
        ofxOMXPlayer* player = omxPlayers[i];

        if( i == 1)
        {
            
            ofPushStyle();
            ofSetColor(ofColor::orange);
            ofRectangle drawRect = player->settings.directDrawRectangle;
            drawRect.width+=10;
            drawRect.height+=10;
            ofDrawRectangle(drawRect);
            ofPopStyle();
            
            player->setLayer(0);
            player->setAlpha(128);
            ofDrawBitmapStringHighlight(player->getInfo(), drawRect.x, drawRect.y, ofColor(ofColor::black, 90), ofColor::yellow);

            
        }else
        {
            //player->setAlpha(90);

            player->setLayer(-1);
        }
    }
#if 0
    for (int i=0; i<omxPlayers.size(); i++) 
    {
        
        ofxOMXPlayer* player = omxPlayers[i];
        float width = player->getWidth()*0.5;
        float height = player->getHeight()*0.5;

         ofRectangle drawRect(width*i,
                             height*i,
                             width,
                             height);
        
        drawRect.x = (ofGetFrameNum()%ofGetWidth());
        //drawRect.y = (height*i)+0.5;
        
        //player->draw(drawRect); 
        
        player->draw(drawRect.x, drawRect.y, drawRect.width, drawRect.height); 
        player->setAlpha((ofGetFrameNum()%255)); 
        ofPushStyle();
            ofSetColor(ofColor::orange);
            drawRect.width+=10;
            drawRect.height+=10;
            ofDrawRectangle(drawRect);
        ofPopStyle();
    }
#endif
    /*
    ofRectangle drawRect1((ofGetFrameNum()%ofGetWidth()), 0, 640, 360);
    omxPlayers[0]->draw(drawRect1);
    omxPlayers[0]->setAlpha(128);
    
    ofRectangle drawRect2(ofRandom(ofGetWidth()), 360, 640, 360);
    omxPlayers[1]->draw(drawRect2);*/

}


