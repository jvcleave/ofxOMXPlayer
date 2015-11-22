#pragma once

#include "ofMain.h"
#include "ofRPIVideoPlayer.h"

class ofApp : public ofBaseApp{

public:

    void setup();
    void update();
    void draw();
        
    ofRPIVideoPlayer rpiVideoPlayer;

    ofTexture pixelOutput;
};

