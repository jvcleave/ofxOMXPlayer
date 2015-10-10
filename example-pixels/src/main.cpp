#include "ofMain.h"
#include "pixelsApp.h"

int main()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp( new pixelsApp());
}