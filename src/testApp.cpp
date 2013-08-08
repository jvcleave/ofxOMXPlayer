#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	
	
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
	
	//Somewhat like ofFboSettings we may have a lot of options so this is the current model
	ofxOMXPlayerSettings settings;
	settings.videoPath = videoPath;
	settings.useHDMIForAudio = true; //default
	settings.enableTexture = true;	//default
	settings.enableLooping = true; //default
	
	
	
	if (settings.enableTexture)
	{
		doShader = true;
		if (doShader) 
		{
			ofEnableAlphaBlending();
			
			shader.load("PostProcessing.vert", "PostProcessing.frag", "");
			
			fbo.allocate(ofGetWidth(), ofGetHeight());
			fbo.begin();
			ofClear(0, 0, 0, 0);
			fbo.end();
			
		}
	}
	
	
	omxPlayer.setup(settings);
	
}

//--------------------------------------------------------------
void testApp::update()
{
	if(!omxPlayer.isPlaying() || !omxPlayer.isTextureEnabled)
	{
		return;
	}
	
	if (doShader) 
	{
		updateFbo();
	}
}

void testApp::updateFbo()
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

//--------------------------------------------------------------
void testApp::draw(){
	if(!omxPlayer.isPlaying() && !omxPlayer.isTextureEnabled)
	{
		return;
	}
	
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

	stringstream info;
	info << "APP FPS: "+ ofToString(ofGetFrameRate());
	info <<"\n" <<	"MEDIA TIME: "			<< omxPlayer.getMediaTime();
	info <<"\n" <<	"DIMENSIONS: "			<< omxPlayer.getWidth()<<"x"<<omxPlayer.getHeight();
	info <<"\n" <<	"DURATION: "			<< omxPlayer.getDuration();
	info <<"\n" <<	"TOTAL FRAMES: "		<< omxPlayer.getTotalNumFrames();
	info <<"\n" <<	"CURRENT FRAME: "		<< omxPlayer.getCurrentFrame();
	info <<"\n" <<	"REMAINING FRAMES: "	<< omxPlayer.getTotalNumFrames() - omxPlayer.getCurrentFrame();
	info <<"\n" <<	"CURRENT VOLUME: "		<< omxPlayer.getVolume();
	
	
	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){

	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){


}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}


//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

