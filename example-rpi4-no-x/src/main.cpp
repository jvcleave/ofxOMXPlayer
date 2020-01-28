#include "ofMain.h"
#include "ofApp.h"
#include "bcm_host.h" 
#include "ofAppNoWindow.h"

int main()
{
    ofSetLogLevel(OF_LOG_VERBOSE);

    ofAppNoWindow window;
    ofSetupOpenGL(&window, 1280, 720, OF_WINDOW);
    
    // and uncomment this line
    // ofSetupOpenGL(300, 300, OF_WINDOW);
    ofRunApp(new ofApp());
    
}
