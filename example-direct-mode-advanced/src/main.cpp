#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "ofAppEGLWindow.h"


class ofApp : public ofBaseApp
{
    
public:
    
    
    ofxOMXPlayer player1;
    ofxOMXPlayer player2;
    
    
    void setup()
    {
        
        ofxOMXPlayerSettings settings1;
        settings1.videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);
        settings1.enableTexture = false;  
        settings1.layer = 1;
        player1.setup(settings1);
        
        ofxOMXPlayerSettings settings2;
        settings2.videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_2.mov", true);
        settings2.enableTexture = false; 
        settings2.layer = 3;
        
        player2.setup(settings2);
        
    }
    
    
    //--------------------------------------------------------------
    void update()
    {
        
    }
    
    
    //--------------------------------------------------------------
    void draw(){
        
        
        
        int alpha = ofGetFrameNum()%255;
        
        ofColor bgColor(ofColor::black, 0);
        ofBackground(bgColor);
        
        
        player1.setAlpha(alpha);
        player1.draw(0, 0, player1.getWidth(), player1.getHeight());
        ofDrawBitmapStringHighlight(player1.getInfo(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
        
        
        
        float scaleFactor = 0.5;
        float width = player2.getWidth()*scaleFactor;
        float height = player2.getHeight()*scaleFactor;
        
        ofRectangle drawRect(width,
                             height,
                             width,
                             height);
        
        //animate
        drawRect.x = (ofGetFrameNum()%ofGetWidth());
        
        player2.draw(drawRect.x, drawRect.y, drawRect.width, drawRect.height); 
        ofPushStyle();
        ofColor orangeBG(ofColor::orange, alpha);
        ofSetColor(orangeBG);
        drawRect.scaleFromCenter(1.25);
        
        ofDrawRectangle(drawRect);
        ofPopStyle();
        
        
    }
};

int main()
{
    ofSetLogLevel(OF_LOG_VERBOSE);

    ofGLESWindowSettings windowSettings;
    windowSettings.setSize(1280, 720);
    
    ofAppEGLWindow::Settings settings(windowSettings);
    settings.layer = 2;
    settings.eglWindowOpacity = 0;
    
    /*
     IN ORDER FOR THIS TO WORK WITH 0.10.0 YOU MUST CHANGE THIS LINE TO 
     dispman_alpha.flags = DISPMANX_FLAGS_ALPHA_FROM_SOURCE;
     https://github.com/openframeworks/openFrameworks/blob/0.10.0/libs/openFrameworks/app/ofAppEGLWindow.cpp#L1913
     */
    
    //My custom OF mod - ignore
    //settings.alphaFlags = DISPMANX_FLAGS_ALPHA_FROM_SOURCE;

    ofCreateWindow(settings);   
    ofRunApp( new ofApp()); 
    
}
