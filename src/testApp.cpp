#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);	
	omxPlayer.setup("/opt/vc/src/hello_pi/hello_video/test.h264");
}

//--------------------------------------------------------------
void testApp::update()
{
	if(!omxPlayer.isReady)
	{
		return;
	}else 
	{
		omxPlayer.update();
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	if(!omxPlayer.isReady)
	{
		return;
	}
	omxPlayer.draw();
	ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 200, 200, ofColor::black, ofColor::yellow);

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

