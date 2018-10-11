#include "ofMain.h"
#include "ofApp.h"
#include "ofAppEGLWindow.h"
   
int main()
{
    ofSetLogLevel(OF_LOG_VERBOSE);

    
    ofGLESWindowSettings windowSettings;
    windowSettings.setSize(1280, 720);
    
    ofAppEGLWindow::Settings settings(windowSettings);
    
    settings.eglWindowOpacity = 127;
    settings.frameBufferAttributes[EGL_DEPTH_SIZE]   = 0; // 0 bits for depth
    settings.frameBufferAttributes[EGL_STENCIL_SIZE] = 0; // 0 bits for stencil
    
    ofCreateWindow(settings);   
    ofRunApp( new ofApp());
}
