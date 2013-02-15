#include "testApp.h"


bool doShader = true;

//--------------------------------------------------------------
void testApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	if (doShader) 
	{
		shader.load("PostProcessing.vert", "PostProcessing.frag", "");
		
		fbo.allocate(ofGetWidth(), ofGetHeight());
		fbo.begin();
		ofClear(0, 0, 0, 0);
		fbo.end();
		ofEnableAlphaBlending();
	}
	
	
	string videoPath = "/opt/vc/src/hello_pi/hello_video/test.h264";
	//videoPath = "/home/pi/videos/";
//	videoPath += "fingers_photo_jpeg.mov";
//	videoPath += "fingers_sorenson.mov";
//	videoPath += "fingers.mp4";
//	videoPath += "london_320x180.mp4";
//	videoPath += "london_320x240.mov";
//	videoPath += "sorted_1280x720.mp4";
//	videoPath += "super8_vimeo_480x270.mp4";
	
	
	
	omxPlayer.setup(videoPath);
}

//--------------------------------------------------------------
void testApp::update()
{
	if(!omxPlayer.isReady)
	{
		return;
	}
	omxPlayer.update();
	if (doShader) 
	{
		updateFbo();
	}
	
	
}

//--------------------------------------------------------------
void testApp::draw(){
	if(!omxPlayer.isReady)
	{
		return;
	}
	
	if (doShader) 
	{
		fbo.draw(0, 0);
	}else 
	{
		omxPlayer.draw();
	}

	
	ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 200, 200, ofColor::black, ofColor::yellow);

}

void testApp::updateFbo()
{
	fbo.begin();
		ofClear(0, 0, 0, 0);
		shader.begin();
			shader.setUniformTexture("tex0", omxPlayer.textureSource, omxPlayer.texture);
			shader.setUniform1f("time", ofGetElapsedTimef());
			shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
			ofRect(0, 0, ofGetWidth(), ofGetHeight());
		shader.end();
	fbo.end();
}
void testApp::exit(){
	omxPlayer.close();
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

