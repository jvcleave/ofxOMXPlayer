#include "developApp.h"

void developApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
	ConsoleListener* thread = (ConsoleListener*) e.listener;
	thread->lock();
		keyPressed((int)e.character);
	thread->unlock();
}
//--------------------------------------------------------------
void developApp::setup()
{
	ofSetVerticalSync(false);
	doRandomSelect		= true;
	isClosing				= false;
	
	usingTexturePlayer		= false;
	
	videoPath = ofToDataPath("big_buck_bunny_MpegStreamclip_720p_h264_50Quality_48K_256k_AAC.mov", true);

	
	/* to get the videos I am testing run command:
	 * $wget -r -nd -P /home/pi/videos http://www.jvcref.com/files/PI/video/
	 */
	
	
	//this will let us just grab a video without recompiling
	ofDirectory currentVideoDirectory("/home/pi/videos/current");
	if (currentVideoDirectory.exists()) 
	{
		//option to put multiple videos in folder to test
		currentVideoDirectory.listDir();
		files = currentVideoDirectory.getFiles();
		if (files.size()>0) 
		{
			if (doRandomSelect && files.size()>1) {
				videoPath = files[ofRandom(files.size())].path();
			}else 
			{
				videoPath = files[0].path();
			}
		}		
	}
	
	ofLogVerbose() << "using videoPath : " << videoPath;
	
	doTextures	= true;
	doShader	= false;
	if (doShader || doTextures) 
	{
		usingTexturePlayer = true;
		createTexturePlayer();
	}else 
	{
		createNonTexturePlayer();
	}
	
	consoleListener.setup(this);
	ofHideCursor();
	doWriteImage = false;
}

void developApp::createNonTexturePlayer()
{
	ofLogVerbose() << "createNonTexturePlayer";
	
	settings.videoPath = videoPath;
	settings.enableTexture = false;
	omxPlayer.setup(settings);
}
void developApp::createTexturePlayer()
{
	//ofSetLogLevel(OF_LOG_VERBOSE); set in main.cpp
	ofEnableAlphaBlending();
	if (doShader) 
	{
		shader.load("PostProcessing.vert", "PostProcessing.frag", "");
		
		fbo.allocate(ofGetWidth(), ofGetHeight());
		fbo.begin();
			ofClear(0, 0, 0, 0);
		fbo.end();
		
	}
	settings.videoPath = videoPath;
	omxPlayer.setup(settings);
}
//--------------------------------------------------------------
void developApp::update()
{
	if (!usingTexturePlayer) 
	{
		return;
	}
	if(omxPlayer.isPlaying())
	{
		if (usingTexturePlayer && doShader) 
		{
			updateFbo();
			if (doWriteImage) 
			{
				string path = ofToDataPath(ofGetTimestampString()+".png", true);
				ofPixels pixels;
				pixels.allocate(ofGetWidth(), ofGetHeight(), OF_PIXELS_RGBA);
				fbo.readToPixels(pixels);
				ofSaveImage(pixels, path);
				doWriteImage = false;
			}
		}
		
	}
}

//--------------------------------------------------------------
void developApp::draw(){
	if (isClosing) {
		return;
	}
	if (!usingTexturePlayer) 
	{
		return;
	}
	if(!omxPlayer.isPlaying())
	{
		return;
	}
	
	if (doShader) 
	{
		fbo.draw(0, 0);
		
	}else 
	{
		omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
		//omxPlayer.draw(0, 0, omxPlayer.getWidth()/4, omxPlayer.getHeight()/4);
	}
	
	stringstream info;
	
	info << "APP FPS: "+ ofToString(ofGetFrameRate());
	
	
	info <<"\n" <<	"MEDIA TIME: "			<< omxPlayer.getMediaTime();
	info <<"\n" <<	"OF DIMENSIONS: "		<< ofGetWidth()<<"x"<<ofGetHeight();
	info <<"\n" <<	"DIMENSIONS: "			<< omxPlayer.getWidth()<<"x"<<omxPlayer.getHeight();
	info <<"\n" <<	"DURATION: "			<< omxPlayer.getDuration();
	info <<"\n" <<	"TOTAL FRAMES: "		<< omxPlayer.getTotalNumFrames();
	info <<"\n" <<	"CURRENT FRAME: "		<< omxPlayer.getCurrentFrame();
	info <<"\n" <<	"REMAINING FRAMES: "	<< omxPlayer.getTotalNumFrames() - omxPlayer.getCurrentFrame();

	info <<"\n" <<	"CURRENT VOLUME: "		<< omxPlayer.getVolume();
	
	info <<"\n" <<	"KEYS:";
	info <<"\n" <<	"p to Toggle Pause";
	info <<"\n" <<	"b to Step frame forward";
	if (usingTexturePlayer) 
	{
		info <<"\n" <<	"s to Toggle Shader";
	}
	info <<"\n" <<	"1 to Decrease Volume";
	info <<"\n" <<	"2 to Increase Volume";
	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(0, 0, 0, 90), ofColor::yellow);
	
	
}

void developApp::updateFbo()
{
	if (isClosing) {
		return;
	}
	if(doShader)
	{
		fbo.begin();
			ofClear(0, 0, 0, 0);
				shader.begin();
				shader.setUniformTexture("tex0", omxPlayer.getTextureReference(), omxPlayer.textureID);
				shader.setUniform1f("time", ofGetElapsedTimef());
				shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
				ofRect(0, 0, ofGetWidth(), ofGetHeight());
			shader.end();
		fbo.end();
	}
	
}

void developApp::exit()
{
	ofLogVerbose() << "developApp::exit";
	isClosing = true;
	
}
//--------------------------------------------------------------
void developApp::keyPressed  (int key){
	 
	ofLogVerbose() << "key received!";
	switch (key) 
	{
		case 'j':
		{
			ofLogVerbose() << "will write image";
			doWriteImage = true;
			doShader = true; //fbo only used with shader in this app
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
			if (usingTexturePlayer)
			{
				doShader = !doShader;
				ofLogVerbose() << "doShader " << doShader;
			}
			break;
		}

		case '1':
		{
			
			ofLogVerbose() << "decreaseVolume";
			omxPlayer.decreaseVolume();
			break;
		}
		case '2':
		{
			ofLogVerbose() << "increaseVolume";
			omxPlayer.increaseVolume();
			break;
		}
			
		case 'b':
		{
			ofLogVerbose() << "stepFrameForward";
			omxPlayer.stepFrameForward();
			break;
		}
			
		default:
		{
			break;
		}	
	}
	
}
