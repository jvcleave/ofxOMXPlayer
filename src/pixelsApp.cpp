#include "pixelsApp.h"

bool hasUpdatedImage = false;
//--------------------------------------------------------------
void pixelsApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(false);
	doSaveImage = false;
	doUpdatePixels = true;
	string videoPath = ofToDataPath("big_buck_bunny_MpegStreamclip_720p_h264_50Quality_48K_256k_AAC.mov", true);
	
	//this will let us just grab a video without recompiling
	ofDirectory currentVideoDirectory("/home/pi/videos/current");
	if (currentVideoDirectory.exists()) 
	{
		currentVideoDirectory.listDir();
		vector<ofFile> files = currentVideoDirectory.getFiles();
		if (files.size()>0) 
		{
			videoPath = files[0].path();
		}		
	}
	consoleListener.setup(this);
	omxPlayer.loadMovie(videoPath);
	
}

//--------------------------------------------------------------
void pixelsApp::update()
{
	if (doSaveImage ) 
	{
		doSaveImage = false;
		omxPlayer.saveImage();
	}
	if (doUpdatePixels) 
	{
		//doUpdatePixels = false;
		
		omxPlayer.updatePixels();
		image.setFromPixels(GlobalEGLContainer::getInstance().pixels);
		
		hasUpdatedImage = true;
	}
	
	
}


//--------------------------------------------------------------
void pixelsApp::draw(){
	if(!omxPlayer.isPlaying() && !omxPlayer.isTextureEnabled)
	{
		return;
	}
	
	omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
	if(hasUpdatedImage) image.draw(0, 0, 400, 300);
	
	stringstream info;
	info << "APP FPS: "+ ofToString(ofGetFrameRate());
	info <<"\n" <<	"MEDIA TIME: "			<< omxPlayer.getMediaTime();
	info <<"\n" <<	"DIMENSIONS: "			<< omxPlayer.getWidth()<<"x"<<omxPlayer.getHeight();
	info <<"\n" <<	"DURATION: "			<< omxPlayer.getDuration();
	info <<"\n" <<	"TOTAL FRAMES: "		<< omxPlayer.getTotalNumFrames();
	info <<"\n" <<	"CURRENT FRAME: "		<< omxPlayer.getCurrentFrame();
	info <<"\n" <<	"REMAINING FRAMES: "	<< omxPlayer.getTotalNumFrames() - omxPlayer.getCurrentFrame();
	info <<"\n" <<	"CURRENT VOLUME: "		<< omxPlayer.getVolume();
	info <<"\n" <<	"Press u to Update Pixels: " << doUpdatePixels;
	info <<"\n" <<	"Press p to save Image";
	
	ofDrawBitmapStringHighlight(info.str(), 600, 60, ofColor(ofColor::black, 90), ofColor::yellow);
}

//--------------------------------------------------------------
void pixelsApp::keyPressed  (int key)
{
	if(key == 's')
	{
		doSaveImage = true;	
	}
	
	if(key == 'u')
	{
		doUpdatePixels = !doUpdatePixels;	
	}
}

void pixelsApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
	keyPressed((int)e.character);
}