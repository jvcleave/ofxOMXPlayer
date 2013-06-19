#include "developApp.h"

void developApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
	keyPressed((int)e.character);
}
bool usingTexturePlayer = false;
//--------------------------------------------------------------
void developApp::setup()
{
	videoPath = "/opt/vc/src/hello_pi/hello_video/test.h264";
	/* to get the videos I am testing run command:
	 * $wget -r -nd -P /home/pi/videos http://www.jvcref.com/files/PI/video/
	 */
	
	
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
	ofLogVerbose() << "using videoPath : " << videoPath;
	
	doTextures = false;
	doShader = false;
	doPause = false;
	if (doShader || doTextures) 
	{
		usingTexturePlayer = true;
		initOMXPlayer();
	}else 
	{
		initVideoPlayer();
	}

	consoleListener.setup(this);
}
void developApp::initVideoPlayer()
{
	omxVideoPlayer.loadMovie(videoPath);
}
void developApp::initOMXPlayer()
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
	

	omxPlayer.loadMovie(videoPath);
}
//--------------------------------------------------------------
void developApp::update()
{
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
	if(!omxPlayer.isPlaying() && !usingTexturePlayer)
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
	
	info << "\n" <<" PAUSED: ";
	if (doPause) 
	{
		info << "TRUE";
	}else 
	{
		info << "FALSE";
	}
	info <<"\n" <<	"MEDIA TIME: "			<< omxPlayer.getMediaTime();
	info <<"\n" <<	"DIMENSIONS: "			<< omxPlayer.getWidth()<<"x"<<omxPlayer.getHeight();
	info <<"\n" <<	"DURATION: "			<< omxPlayer.getDuration();
	info <<"\n" <<	"TOTAL FRAMES: "		<< omxPlayer.getTotalNumFrames();
	info <<"\n" <<	"CURRENT FRAME: "		<< omxPlayer.getCurrentFrame();
	info <<"\n" <<	"REMAINING FRAMES: "	<< omxPlayer.getTotalNumFrames() - omxPlayer.getCurrentFrame();

	info <<"\n" <<	omxPlayer.getVideoDebugInfo() << endl;
	
	info <<"\n" <<	"KEYS:";
	info <<"\n" <<	"p to toggle Pause";
	info <<"\n" <<	"1 to send Play command";
	info <<"\n" <<	"2 to send Stop command";
	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(0, 0, 0, 90), ofColor::yellow);
	
	
}

void developApp::updateFbo()
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

void developApp::exit()
{
	bool DO_HARD_EXIT = false;
	if(DO_HARD_EXIT)
	{
		ofLogVerbose() << "developApp::exiting hard";
		atexit(0);
	}else 
	{
		doShader = false;
		ofSleepMillis(20);
		omxPlayer.close();
	}
	
}
//--------------------------------------------------------------
void developApp::keyPressed  (int key){
	
	ofLogVerbose() << key << "received!";
	
	switch (key) 
	{
		case 'p':
		{
			
			doPause = !doPause;
			ofLogVerbose() << "SENDING PAUSE STATE: " << doPause;
			omxPlayer.setPaused(doPause);
			break;
		}
		case '1':
		{
			ofLogVerbose() << "SENDING PLAY COMMAND";
			omxPlayer.play();
			break;
		}
		case '2':
		{
			ofLogVerbose() << "SENDING STOP COMMAND";
			omxPlayer.stop();
			break;
		}
			
		default:
		{
			break;
		}	
	}
	
}

//--------------------------------------------------------------
void developApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void developApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void developApp::mouseDragged(int x, int y, int button){
	
	
}

//--------------------------------------------------------------
void developApp::mousePressed(int x, int y, int button){
	
}

//--------------------------------------------------------------
void developApp::mouseReleased(int x, int y, int button){
	
}


//--------------------------------------------------------------
void developApp::windowResized(int w, int h){
	
}

//--------------------------------------------------------------
void developApp::gotMessage(ofMessage msg){
	
}

//--------------------------------------------------------------
void developApp::dragEvent(ofDragInfo dragInfo){ 
	
}

