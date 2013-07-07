#include "developApp.h"

void developApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
	keyPressed((int)e.character);
}
//--------------------------------------------------------------
void developApp::setup()
{
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
	doShader	= true;
	isShaderEnabled = doShader;
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
	
	if (doShader && isShaderEnabled) 
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
	if(isShaderEnabled)
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
	 
	ofLogVerbose() << key << " received!";
	if (key == 0x5b44) {
		ofLogVerbose() << "RIGHT KEY PRESSED";
	}
	
	switch (key) 
	{
		case 'p':
		{
			omxPlayer.setPaused(!omxPlayer.isPaused());
			break;
		
		}
			
		case 's':
		{
			if (doShader && usingTexturePlayer)
			{
				isShaderEnabled = !isShaderEnabled;
			}
			break;
		}

		case '1':
		{
			
			omxPlayer.decreaseVolume();
			break;
		}
		case '2':
		{
			omxPlayer.increaseVolume();
			break;
		}
			
		case 'b':
		{
			
			omxPlayer.stepFrameForward();
			break;
		}
			
		default:
		{
			break;
		}	
	}
	
}
