#include "ofMain.h"
#include "multiPlayer.h"

int main()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp( new multiPlayer());
}
