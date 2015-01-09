#include "ofMain.h"
#include "ofApp.h"
#include "ofGLProgrammableRenderer.h"

int main()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetLogLevel("ofThread", OF_LOG_ERROR);
	ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp( new ofApp());
}