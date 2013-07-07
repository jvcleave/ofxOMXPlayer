#include "ofMain.h"

//#define USE_DEVELOP_APP

#ifdef USE_DEVELOP_APP
	#include "developApp.h"
#warning "!!!!!!! YOU ARE USING THE developApp WHEN YOU PROBABLY WANT THE testApp !!!!!!!!"
#else
	#include "testApp.h"
#endif

#ifdef PROGRAMMABLE_PRESENT
	#include "ofGLProgrammableRenderer.h"
#else
	#include "ofGLES2Renderer.h"
#endif

//========================================================================
int main( ){
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	#ifdef PROGRAMMABLE_PRESENT
		ofSetCurrentRenderer(ofPtr<ofBaseRenderer>(new ofGLProgrammableRenderer()));
	#else
		ofSetCurrentRenderer(ofPtr<ofBaseRenderer>(new ofGLES2Renderer()));
	#endif
	
	ofSetupOpenGL(1280, 720, OF_WINDOW);

	#ifdef USE_DEVELOP_APP
		ofRunApp( new developApp());
	#else
		ofRunApp( new testApp());
	#endif
}
