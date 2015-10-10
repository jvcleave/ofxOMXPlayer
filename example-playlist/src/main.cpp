#include "ofMain.h"
#include "playlistApp.h"

int main()
{
	ofSetLogLevel(OF_LOG_NOTICE);
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp( new playlistApp());
}