#include "testApp.h"


bool doShader = true;
bool doPause = false;
bool doTestPausing = false;
//pausing needed for stop/play testing
	int pauseTestCounter = 0;
	bool doTestStop = true;
	bool doTestPlay = true;

bool DO_HARD_EXIT = true;

bool doTestSeeking = false; //needs work
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
	
/* to get these videos run command:
 * wget -r -nd -P /home/pi/videos http://www.jvcref.com/files/PI/video/
*/
	string videoPath = "/opt/vc/src/hello_pi/hello_video/test.h264";
	videoPath = "/home/pi/videos/";
	
//	videoPath += "fingers_photo_jpeg.mov";
//	videoPath += "fingers_sorenson.mov";
//	videoPath += "fingers.mp4";
//	videoPath += "london_320x180.mp4";
//	videoPath += "london_320x240.mov";
//	videoPath += "sorted_1280x720.mp4";
//	videoPath += "1_g1_1280x720.mp4";
//	videoPath += "1_g1_480x270.mp4";
//	videoPath += "London_1920x1080.mov";
//	videoPath += "London_480x270_MpegStreamClip.mov";
	videoPath += "Cars720p_MpegStreamClip.mov";
	
	//from http://www.cnx-software.com/2013/01/26/raspberry-pi-now-has-experimental-support-for-vp6-vp8-mjpeg-and-ogg-theora-video-codecs/
	//P1020080.MOV kinda works - will try fix above later
	
	//videoPath += "P1020080.MOV";
	//videoPath += "trailer_VP6.flv";
	//videoPath += "trailer_400p.ogg";
	//videoPath += "big_buck_bunny_trailer_480p.webm";
	
//	videoPath += "super8_vimeo_480x270.mp4";
	
	
	omxPlayer.loadMovie(videoPath);
}

//--------------------------------------------------------------
void testApp::update()
{
	if(!omxPlayer.isPlaying())
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
		omxPlayer.draw(0, 0, omxPlayer.getWidth()/4, omxPlayer.getHeight()/4);
	}
	//test Pausing
	int millisecondsBeforeWeSeeSomething = 4000;
	if (doTestPausing && ofGetElapsedTimeMillis()> millisecondsBeforeWeSeeSomething) 
	{
		if (ofGetFrameNum() % 300 == 0) 
		{
			doPause = !doPause;
			ofLogVerbose() << "Setting doPause to " << doPause;
			omxPlayer.setPaused(doPause);
			if (doTestStop) //give a few pauses then stop player
			{
				pauseTestCounter++; 
				if (pauseTestCounter == 3) 
				{
					omxPlayer.stop();
					if (doTestPlay) 
					{
						ofSleepMillis(2000);
						omxPlayer.play();
						doTestPausing = false;
						doPause = false;
						ofLogVerbose() << "testing over";
					}
				}
			}
			
		}
		
	}
	if (doTestSeeking) 
	{
		if (ofGetElapsedTimeMillis()> millisecondsBeforeWeSeeSomething)
		{
			if (ofGetFrameNum() % 50 == 0) 
			{
				omxPlayer.setPosition(ofRandom(1.0f, 100.0f));
			}
		}
		
	}
	stringstream info;
	
	info << "APP FPS: "+ ofToString(ofGetFrameRate());
	
	if (doTestPausing)
	{
		info << "\n" <<" PAUSED: ";
		if (doPause) 
		{
			info << "TRUE";
		}else 
		{
			info << "FALSE";
		}	
	}
	info <<"\n" <<	"MEDIA TIME: "			<< omxPlayer.getMediaTime();
	info <<"\n" <<	"DIMENSIONS: "			<< omxPlayer.getWidth()<<"x"<<omxPlayer.getHeight();
	info <<"\n" <<	"DURATION: "			<< omxPlayer.getDuration();
	info <<"\n" <<	"TOTAL FRAMES: "		<< omxPlayer.getTotalNumFrames();
	
	ofDrawBitmapStringHighlight(info.str(), 200, 200, ofColor::black, ofColor::yellow);


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

void testApp::exit()
{
	if(DO_HARD_EXIT)
	{
		ofLogVerbose() << "testApp::exiting hard";
		atexit(0);
	}else 
	{
		omxPlayer.close();
	}
	
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

