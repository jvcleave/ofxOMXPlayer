#include "ofMain.h"
#include "ofApp.h"



int main()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofGLESWindowSettings settings;
    settings.width = 1280;
    settings.height = 720;
    settings.setGLESVersion(2);
    ofCreateWindow(settings);
    ofRunApp( new ofApp());
}