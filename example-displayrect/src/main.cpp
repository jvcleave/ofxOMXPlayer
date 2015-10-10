#include "ofMain.h"
#include "ofApp.h"
#if (OF_VERSION_MINOR != 9)
#include "ofGLProgrammableRenderer.h"
#endif


int main()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
#if (OF_VERSION_MINOR == 9)
    ofGLESWindowSettings settings;
    settings.width = 1280;
    settings.height = 720;
    settings.setGLESVersion(2);
    ofCreateWindow(settings);
#else
    ofSetLogLevel("ofThread", OF_LOG_ERROR);
    ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
    ofSetupOpenGL(1280, 720, OF_WINDOW);
#endif
    ofRunApp( new ofApp());
}