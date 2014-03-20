#include "pixelsApp.h"

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
		//ofImage version
		//pixelOutput.setFromPixels(omxPlayer.getPixels(), omxPlayer.getWidth(), omxPlayer.getHeight(), OF_IMAGE_COLOR_ALPHA, true);
		if (!pixelOutput.isAllocated()) 
		{
			pixelOutput.allocate(omxPlayer.getWidth(), omxPlayer.getHeight(), GL_RGBA);
		}
		pixelOutput.loadData(omxPlayer.getPixels(), omxPlayer.getWidth(), omxPlayer.getHeight(), GL_RGBA);
	}
	
	
}


//--------------------------------------------------------------
void pixelsApp::draw(){
	if(!omxPlayer.isTextureEnabled)
	{
		return;
	}
	
	omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
	pixelOutput.draw(20, 20, omxPlayer.getWidth()/2, omxPlayer.getHeight()/2);
	
	stringstream info;
	info <<"\n" <<	"Press u to Update Pixels: " << doUpdatePixels;
	info <<"\n" <<	"Press s to save Image";
	
	ofDrawBitmapStringHighlight(omxPlayer.getInfo() + info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);

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