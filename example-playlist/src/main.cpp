#include "ofMain.h"
#include "playlistApp.h"
//#include "ofGLProgrammableRenderer.h"

int main()
{
	ofSetLogLevel(OF_LOG_NOTICE);
	//ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp( new playlistApp());
}