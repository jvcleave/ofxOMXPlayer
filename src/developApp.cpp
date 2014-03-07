#include "developApp.h"

bool hasFrameChanged = false;
int previousFrameNumber =0;
void developApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
	//ConsoleListener* thread = (ConsoleListener*) e.listener;
	keyPressed((int)e.character);
}


void developApp::onVideoEnd(ofxOMXPlayerListenerEventData& e)
{
	ofLogVerbose(__func__) << "at: " << ofGetElapsedTimeMillis();
}
void developApp::onVideoLoop(ofxOMXPlayerListenerEventData& e)
{
	ofLogVerbose(__func__) << "at: " << ofGetElapsedTimeMillis();
}


//--------------------------------------------------------------
void developApp::setup()
{
	ofSetVerticalSync(false);
	
	consoleListener.setup(this);
	ofHideCursor();
	doWriteImage		= false;
	
	
	string videoPath = ofToDataPath("big_buck_bunny_MpegStreamclip_720p_h264_50Quality_48K_256k_AAC.mov", true);

	
	/* to get the videos I am testing run command:
	 * $wget -r -nd -P /home/pi/videos http://www.jvcref.com/files/PI/video/
	 */
	
	
	//this will let us just grab a video without recompiling
	ofDirectory currentVideoDirectory("/home/pi/videos/current");
	bool doRandomSelect		= true;
	if (currentVideoDirectory.exists()) 
	{
		//option to put multiple videos in folder to test
		currentVideoDirectory.listDir();
		vector<ofFile> files = currentVideoDirectory.getFiles();
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
	settings.videoPath = videoPath;
	settings.listener = this; //this app extends ofxOMXPlayerListener so it will receive events ;
	settings.enableLooping = true;
	doTextures	= true;
	doShader	= false;
	
	
	//settings.enableAudio = false; //default true
	
	if (doShader) 
	{
		loadShader();		
	}else 
	{
		settings.displayRect.x = 100;
		settings.displayRect.y = 200;
		settings.displayRect.width = 400;
		settings.displayRect.height = 300;
	}

	
	if (doShader || doTextures) 
	{
		settings.enableTexture = true;
	}else 
	{
		settings.enableTexture = false;
	}
	omxPlayer.setup(settings);
	
}

void developApp::loadShader()
{
	ofEnableAlphaBlending();
	if (!shader.isLoaded()) 
	{
		shader.load("PostProcessing.vert", "PostProcessing.frag", "");
		
		fbo.allocate(ofGetWidth(), ofGetHeight());
		fbo.begin();
			ofClear(0, 0, 0, 0);
		fbo.end();
		
	}
}

//--------------------------------------------------------------
void developApp::update()
{
	if (!omxPlayer.isPlaying() || !omxPlayer.isTextureEnabled) 
	{
		return;
	}
	
	if (doShader) 
	{
		if (!shader.isLoaded()) 
		{
			loadShader();
		}
		
		fbo.begin();
			ofClear(0, 0, 0, 0);
			shader.begin();
			shader.setUniformTexture("tex0", omxPlayer.getTextureReference(), omxPlayer.getTextureID());
			shader.setUniform1f("time", ofGetElapsedTimef());
			shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
			ofRect(0, 0, ofGetWidth(), ofGetHeight());
			shader.end();
		fbo.end();
		
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

//--------------------------------------------------------------
void developApp::draw(){
	
	
	if (omxPlayer.isTextureEnabled) 
	{
		
		if (doShader) 
		{
			if (!shader.isLoaded()) 
			{
				loadShader();
			}
			
			fbo.draw(0, 0);
			
		}else 
		{
			omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
			//omxPlayer.draw(0, 0, omxPlayer.getWidth()/4, omxPlayer.getHeight()/4);
		}
	}else 
	{
		//omxPlayer.draw(200, 200, omxPlayer.getWidth()/4, omxPlayer.getHeight()/4);
	}

	
	stringstream info;
	info <<"\n" <<  "APP FPS: "+ ofToString(ofGetFrameRate());
	
	
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
	if (settings.enableTexture) 
	{
		info <<"\n" <<	"s to Toggle Shader";
	}
	info <<"\n" <<	"1 to Decrease Volume";
	info <<"\n" <<	"2 to Increase Volume";
	ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(0, 0, 0, 90), ofColor::yellow);
	
	
}

void developApp::exit()
{
	omxPlayer.close();
	ofLogVerbose() << "developApp::exit";
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
			if (settings.enableTexture)
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
			
		case '4':
		{
			ofLogVerbose() << "rewind";
			omxPlayer.rewind();
			break;
		}
			
		case '5':
		{
			ofLogVerbose() << "setNormalSpeed";
			omxPlayer.setNormalSpeed();
			break;
		}
	
		case '6':
		{
			ofLogVerbose() << "fastForward";
			omxPlayer.fastForward();
			break;
		}
		case 'b':
		{
			ofLogVerbose() << "stepFrameForward";
			omxPlayer.stepFrameForward();
			break;
		}
		case 'c':
		{
			omxPlayer.close();
			break;
		}
		default:
		{
			break;
		}	
	}
	
}
