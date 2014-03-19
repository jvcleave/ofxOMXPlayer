#include "playlistApp.h"

//This app is a demo of the ability to play multiple files with the Non-Texture Player
//It requires multiple video files to be in /home/pi/videos/current
//There is a bit of glitching while the files switch

//This also demonstrates the ofxOMXPlayerListener pattern available

//If your app extends ofxOMXPlayerListener you will receive an event when the video ends


void checkForError() {
	GLenum err (glGetError());
	
	while(err!=GL_NO_ERROR) 
	{
		string error;
		
		switch(err) 
		{
			case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
			case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
			case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
			case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
		}
		
		ofLogError() << "error: " << error;
		err=glGetError();
	}
}



bool doLoadNextMovie = false;
void playlistApp::onVideoEnd(ofxOMXPlayerListenerEventData& e)
{
	ofLogVerbose(__func__) << " RECEIVED";
	doLoadNextMovie = true;
}


void playlistApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
	keyPressed((int)e.character);
}



unsigned long long skipTimeStart=0;
unsigned long long skipTimeEnd=0;
unsigned long long amountSkipped =0;
unsigned long long totalAmountSkipped =0;
//--------------------------------------------------------------
void playlistApp::setup()
{
	ofBackground(ofColor::black);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(false);
	consoleListener.setup(this);	
	doShader = false;
	doPixels = false;
	//this will let us just grab a video without recompiling
	ofDirectory currentVideoDirectory("/home/pi/videos/current");
	if (currentVideoDirectory.exists()) 
	{
		currentVideoDirectory.listDir();
		currentVideoDirectory.sort();
		files = currentVideoDirectory.getFiles();
		if (files.size()>0) 
		{
			videoCounter = 0;
			settings.videoPath = files[videoCounter].path();
			settings.useHDMIForAudio = true;	//default true
			settings.enableLooping = false;
			settings.enableTexture = false;		//default true
			if (!settings.enableTexture) 
			{
				settings.displayRect.x = 100;
				settings.displayRect.y = 200;
				settings.displayRect.width = 400;
				settings.displayRect.height = 300;
			}
			
			//settings.enableAudio = !settings.enableAudio; //toggle for testing
			settings.listener = this; //this app extends ofxOMXPlayerListener so it will receive events ;
			omxPlayer.setup(settings);
		}		
	}
}


void playlistApp::loadNextMovie()
{
	if(videoCounter+1<files.size())
	{
		videoCounter++;
	}else
	{
		videoCounter = 0;
	}
	skipTimeStart = ofGetElapsedTimeMillis();
	omxPlayer.loadMovie(files[videoCounter].path());
	skipTimeEnd = ofGetElapsedTimeMillis();
	amountSkipped = skipTimeEnd-skipTimeStart;
	totalAmountSkipped+=amountSkipped;
	doLoadNextMovie = false;
}

//--------------------------------------------------------------
void playlistApp::update()
{
	if (doLoadNextMovie) 
	{
		ofLogVerbose(__func__) << "doing reload";
		//with the texture based player this must be done here - especially if the videos are different resolutions
		
		loadNextMovie();
		if (doShader && shader.isLoaded()) 
		{
			shader.unload();
			loadShader();
		}
		
	}
	if (doShader && settings.enableTexture) 
	{
		if (!shader.isLoaded()) 
		{
			loadShader();
		}
		
		fbo.begin();
			ofClear(0, 0, 0, 0);
			shader.begin();
				shader.setUniformTexture("tex0", omxPlayer.getTextureReference(), omxPlayer.getTextureID());
				shader.setUniform1f("time", ofGetElapsedTimef());
				shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
				omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
			shader.end();
		fbo.end();	
	}
	
	
}


//--------------------------------------------------------------
void playlistApp::draw(){
	
	//ofBackgroundGradient(ofColor::red, ofColor::black, OF_GRADIENT_CIRCULAR);
	
	if(omxPlayer.isTextureEnabled)
	{
		
		
		if (doShader) 
		{
			fbo.draw(0, 0);
		}else 
		{
			omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
			
			//draw a smaller version in the lower right
			int scaledHeight = omxPlayer.getHeight()/4;
			int scaledWidth = omxPlayer.getWidth()/4;
			omxPlayer.draw(ofGetWidth()-scaledWidth, ofGetHeight()-scaledHeight, scaledWidth, scaledHeight);
		}
		if (doPixels) 
		{
			omxPlayer.updatePixels();
			//ofImage version
			//pixelOutput.setFromPixels(omxPlayer.getPixels(), omxPlayer.getWidth(), omxPlayer.getHeight(), OF_IMAGE_COLOR_ALPHA, true);
			if (!pixelOutput.isAllocated() || (omxPlayer.getWidth()!=pixelOutput.getWidth()) ||  (omxPlayer.getHeight()!=pixelOutput.getHeight())) 
			{
				pixelOutput.allocate(omxPlayer.getWidth(), omxPlayer.getHeight(), GL_RGBA);
			}
			pixelOutput.loadData(omxPlayer.getPixels(), omxPlayer.getWidth(), omxPlayer.getHeight(), GL_RGBA);
			int pixelDrawWidth = pixelOutput.getWidth()/2;
			int pixelDrawHeight = pixelOutput.getHeight()/2;
			
			ofPushMatrix();
				ofTranslate(ofGetWidth()-pixelDrawWidth, 0);
				pixelOutput.draw(0, 0, pixelDrawWidth, pixelDrawHeight);
			ofPopMatrix();
		}
		
	}else 
	{
		/*if (ofGetElapsedTimeMillis()>15000) 
		{
			omxPlayer.draw(ofRandom(100, 200), 200, ofRandom(100, ofGetWidth()), ofRandom(300, ofGetHeight()));
		}*/
		
	}

	
	
	
	

	stringstream info;
	info <<"\n" <<	"MILLIS SKIPPED: "		<< amountSkipped;
	info <<"\n" <<	"TOTAL MILLIS SKIPPED: " << totalAmountSkipped;
	info <<"\n" <<	"CURRENT MOVIE: "		<< files[videoCounter].path();
	ofDrawBitmapStringHighlight(omxPlayer.getInfo() + info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
}

//--------------------------------------------------------------
void playlistApp::keyPressed  (int key){

	ofLogVerbose(__func__) << "key: " << key;
	switch (key) 
	{
		case 'n':
		{
			doLoadNextMovie = true;
			break;
		}
		case 'e':
		{
			break;
		}
		case 'x':
		{
			_Exit(0);
			break;
		}
		case 'p':
		{
			ofLogVerbose() << "pause: " << !omxPlayer.isPaused();
			omxPlayer.setPaused(!omxPlayer.isPaused());
			break;
			
		}
		case 's':
		{
			if (settings.enableTexture ) 
			{
				doShader = !doShader;
			}
			break;
		}
		case 'P':
		{
			if (settings.enableTexture ) 
			{
				doPixels = !doPixels;
			}
			break;
		}

	}
}

void playlistApp::loadShader()
{
	ofEnableAlphaBlending();
	if (!shader.isLoaded()) 
	{
		shader.load("shaderExample.vert", "shaderExample.frag", "");
		check_gl_error(__FILE__,__LINE__);
		fbo.allocate(ofGetWidth(), ofGetHeight());
		check_gl_error(__FILE__,__LINE__);
		fbo.begin();
			ofClear(0, 0, 0, 0);
		fbo.end();
	}
}

