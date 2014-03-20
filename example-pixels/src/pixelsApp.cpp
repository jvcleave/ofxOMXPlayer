#include "pixelsApp.h"

//--------------------------------------------------------------
void pixelsApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(false);
	doSaveImage = false;
	doUpdatePixels = true;
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);

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
		//since updatePixels() is expensive it is not automatically called in the player		
		omxPlayer.updatePixels();

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
	info <<"\n" <<	"Press u to toggle doUpdatePixels: " << doUpdatePixels;
	
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