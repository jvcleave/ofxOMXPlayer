

//Ugly but I change these a lot 

//#define USE_DEVELOP_APP
//#define USE_PLAYLIST_APP
//#define USE_TEST_APP
//#define USE_PIXELS_APP
#define USE_MULTIPLAYER_APP


//========================================================================
#ifdef USE_MULTIPLAYER_APP
#warning "!!!!!!! YOU ARE USING THE multiPlayer"

#include "ofMain.h"
#include "multiPlayer.h"
#include "ofGLProgrammableRenderer.h"

int main()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp( new multiPlayer());
}
#endif

//========================================================================
#ifdef USE_TEST_APP
	#warning "!!!!!!! YOU ARE USING THE testApp"

	#include "ofMain.h"
	#include "testApp.h"
	#include "ofGLProgrammableRenderer.h"

	int main()
	{
		ofSetLogLevel(OF_LOG_VERBOSE);
		ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
		ofSetupOpenGL(1280, 720, OF_WINDOW);
		ofRunApp( new testApp());
	}
#endif


//========================================================================
#ifdef USE_DEVELOP_APP
	#warning "!!!!!!! YOU ARE USING THE developApp"

	#include "ofMain.h"
	#include "developApp.h"
	#include "ofGLProgrammableRenderer.h"

	int main()
	{
		ofSetLogLevel(OF_LOG_VERBOSE);
		ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
		ofSetupOpenGL(1920, 1080, OF_WINDOW);
		ofRunApp( new developApp());
	}

#endif


//========================================================================
#ifdef USE_PLAYLIST_APP
	#warning "!!!!!!! YOU ARE USING THE playlistApp"

	#include "ofMain.h"
	#include "playlistApp.h"
	#include "ofGLProgrammableRenderer.h"

	int main()
	{
		ofSetLogLevel(OF_LOG_VERBOSE);
		ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
		ofSetupOpenGL(1920, 1080, OF_WINDOW);
		ofRunApp( new playlistApp());
	}
#endif

//========================================================================
#ifdef USE_PIXELS_APP
#warning "!!!!!!! YOU ARE USING THE pixelsApp"

#include "ofMain.h"
#include "pixelsApp.h"
//#include "ofGLProgrammableRenderer.h"

int main()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	//ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp( new pixelsApp());
}
#endif