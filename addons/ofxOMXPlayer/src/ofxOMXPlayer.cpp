#include "ofxOMXPlayer.h"

ofxOMXPlayer::ofxOMXPlayer()
{
	videoWidth = 0;
	videoHeight =0;
	isMPEG               = false;
	hasVideo           = false;
	packet = NULL;
	moviePath = "moviePath is undefined";
	clock = NULL;
	playerTex = NULL;
	pixels = NULL;
	bPaused = false;
	bPlaying = false;
	duration = 0.0;
	
}
void ofxOMXPlayer::loadMovie(string filepath)
{
	moviePath = filepath; 
	rbp.Initialize();
	omxCore.Initialize();
	
	clock = new OMXClock(); 
	bool doDumpFormat = false;

	if(omxReader.Open(moviePath.c_str(), doDumpFormat))
	{
		
		ofLogVerbose() << "omxReader open moviePath PASS: " << moviePath;
		hasVideo     = omxReader.VideoStreamCount();
		if (hasVideo) 
		{
			ofLogVerbose() << "Video streams detection PASS";
			
			bool hasAudio = false; //not implemented yet
			if(clock->OMXInitialize(hasVideo, hasAudio))
			{
				ofLogVerbose() << "clock Init PASS";
				openPlayer();
				
				
			}else 
			{
				ofLogError() << "clock Init FAIL";
			}
		}else 
		{
			ofLogError() << "Video streams detection FAIL";
		}
	}else 
	{
		ofLogError() << "omxReader open moviePath FAIL: "  << moviePath;
	}
	
}
void ofxOMXPlayer::openPlayer()
{
	omxReader.GetHints(OMXSTREAM_VIDEO, streamInfo);
	videoWidth	= streamInfo.width;
	videoHeight	= streamInfo.height;
	ofLogVerbose() << "SET videoWidth: " << videoWidth;
	ofLogVerbose() << "SET videoHeight: " << videoHeight;	
	generateEGLImage();
	bPlaying = omxPlayerVideo.Open(streamInfo, clock, eglImage);
	if (isPlaying()) 
	{
		if(streamInfo.nb_frames>0 && omxPlayerVideo.GetFPS()>0)
		{
			duration = streamInfo.nb_frames / omxPlayerVideo.GetFPS();
			ofLogVerbose() << "duration SET: " << duration;
		}
		
		ofLogVerbose() << "Opened video PASS";					
	}else 
	{
		ofLogError() << "Opened video FAIL";
	}
}

void ofxOMXPlayer::generateEGLImage()
{
	ofDisableArbTex();
	
	ofAppEGLWindow *appEGLWindow = (ofAppEGLWindow *) ofGetWindowPtr();
	display = appEGLWindow->getEglDisplay();
	context = appEGLWindow->getEglContext();

	
	tex.allocate(videoWidth, videoHeight, GL_RGBA);
	tex.getTextureData().bFlipTexture = true;
	tex.setTextureWrap(GL_REPEAT, GL_REPEAT);
	//textureSource.setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
	//textureSource.setTextureWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	textureID = tex.getTextureData().textureID;
	
	//TODO - should be a way to use ofPixels for the getPixels() functions?
	glEnable(GL_TEXTURE_2D);

	// setup first texture
	int dataSize = videoWidth * videoHeight * 4;
	
	GLubyte* pixelData = new GLubyte [dataSize];
	
	
    memset(pixelData, 0xff, dataSize);  // white texture, opaque
	
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, videoWidth, videoHeight, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
	
	delete[] pixelData;
	
	
	// Create EGL Image
	eglImage = eglCreateImageKHR(
								 display,
								 context,
								 EGL_GL_TEXTURE_2D_KHR,
								 (EGLClientBuffer)textureID,
								 0);
    glDisable(GL_TEXTURE_2D);
	if (eglImage == EGL_NO_IMAGE_KHR)
	{
		ofLogError()	<< "Create EGLImage FAIL";
		return;
	}
	else
	{
		ofLogVerbose()	<< "Create EGLImage PASS";
	}
	/*pixels = new ofPixels();
	pixels->allocate(videoWidth, videoHeight, GL_RGBA);*/
}

//---------------------------------------------------------------------------
//for getting a reference to the texture
ofTexture & ofxOMXPlayer::getTextureReference(){
	if(playerTex == NULL){
		return tex;
	}
	else{
		return *playerTex;
	}
}

void ofxOMXPlayer::update()
{
	if(!isPlaying())
	{
		return;
	}
	
	if(!packet)
	{
		packet = omxReader.Read();
	}
	
	if(packet && omxReader.IsActive(OMXSTREAM_VIDEO, packet->stream_index))
    {
		if(omxPlayerVideo.AddPacket(packet))
		{
			packet = NULL;
		}else 
		{
			OMXClock::OMXSleep(10);
		}
	}else 
	{
		if(packet)
		{
			omxReader.FreePacket(packet);
			packet = NULL;
		}
	}
}
//--------------------------------------------------------
void ofxOMXPlayer::play(){
	ofLogVerbose() << "TODO: not sure what to do with this - reopen the player?";
	/*clock->SetSpeed(OMX_PLAYSPEED_NORMAL);
	clock->OMXStateExecute();
	clock->OMXStart();
	bPlaying = openPlayer();*/
}

//--------------------------------------------------------
void ofxOMXPlayer::stop()
{
	clock->OMXStop();
	clock->OMXStateIdle();
	omxPlayerVideo.Close();
	bPlaying = false;
	ofLogVerbose() << "ofxOMXPlayer::stop called";
}

//---------------------------------------------------------------------------
void ofxOMXPlayer::setPaused(bool _bPause){
	bPaused = _bPause;
	if(bPaused)
	{
		omxPlayerVideo.SetSpeed(OMX_PLAYSPEED_PAUSE);
		clock->OMXPause();
	}else 
	{
		omxPlayerVideo.SetSpeed(OMX_PLAYSPEED_NORMAL);
		clock->OMXResume();
	}

			
}

//---------------------------------------------------------------------------
unsigned char * ofxOMXPlayer::getPixels(){
	if( pixels != NULL ){
		return pixels->getPixels();
	}
	return NULL;	
}

//---------------------------------------------------------------------------
ofPixelsRef ofxOMXPlayer::getPixelsRef(){
	if (pixels == NULL) 
	{
		//TODO figure this out
		ofLogError() << "probably going to crash";
	}
	return *pixels;
}

//------------------------------------
void ofxOMXPlayer::draw(float _x, float _y, float _w, float _h)
{
	if(!isPlaying()) return;
	getTextureReference().draw(_x, _y, _w, _h);	
}

//------------------------------------
void ofxOMXPlayer::draw(float _x, float _y)
{
	if(!isPlaying()) return;
	getTextureReference().draw(_x, _y);
}

//---------------------------------------------------------------------------
float ofxOMXPlayer::getDuration(){
	
	
	return duration;
}
//----------------------------------------------------------
float ofxOMXPlayer::getWidth()
{
	return videoWidth;
}

//----------------------------------------------------------
float ofxOMXPlayer::getHeight()
{
	return videoHeight;
}

//----------------------------------------------------------
bool ofxOMXPlayer::isPaused()
{
	return bPaused;
}

//----------------------------------------------------------
bool ofxOMXPlayer::isPlaying(){
	return bPlaying;
}

void ofxOMXPlayer::close()
{
	if(isPlaying()) 
	 {
		  stop();
	 }
	 if(packet)
	 {
		 omxReader.FreePacket(packet);
		 packet = NULL;
	 }	
		omxReader.Close();

	 if (eglImage !=NULL) 
	 {
		 eglDestroyImageKHR(display, eglImage);
	 }

	omxCore.Deinitialize();
	rbp.Deinitialize();
	ofLogVerbose() << "reached end of ofxOMXPlayer::close";
}
