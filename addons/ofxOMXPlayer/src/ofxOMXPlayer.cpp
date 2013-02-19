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
	nFrames = 0;
	doVideoDebugging = true;
	if (doVideoDebugging) 
	{
		omxPlayerVideo.doDebugging = true;
	}
	speed = 0;
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
		speed = clock->OMXPlaySpeed();
		ofLogVerbose() << "speed : " <<speed;
		if(streamInfo.nb_frames>0 && omxPlayerVideo.GetFPS()>0)
		{
			nFrames = streamInfo.nb_frames;
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
	//omxPlayerVideo.WaitCompletion();
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
string ofxOMXPlayer::getVideoDebugInfo()
{
	if (!doVideoDebugging) 
	{
		return "doVideoDebugging not enabled";
	}
	if (omxPlayerVideo.m_decoder != NULL) 
	{
		return omxPlayerVideo.m_decoder->debugInfo + "\n" + omxPlayerVideo.debugInfo;
	}
	return "NO INFO YET";
}

int ofxOMXPlayer::getSpeed(){
	return speed;
}
//--------------------------------------------------------------------
void ofxOMXPlayer::setSpeed(int rate){
	speed = rate;
	omxReader.SetSpeed(speed);
	clock->OMXSpeed(speed);
	//omxPlayerVideo.SetSpeed(speed*1000);
}

//---------------------------------------------------------------------------
float ofxOMXPlayer::getPosition(){
	//TODO: check if behavior is the same
	return getMediaTime();
}
//---------------------------------------------------------------------------
float ofxOMXPlayer::getDuration(){
	return duration;
}

void ofxOMXPlayer::setPosition(float pct)
{
	if (!isPlaying()) 
	{
		return;
	}
	
	//int m_seek_pos = 0;

	/*ofLogVerbose() << "pct: " << pct;
	ofLogVerbose() << "duration: " << duration;
	ofLogVerbose() << "omxPlayerVideo.GetFPS(): " << omxPlayerVideo.GetFPS();
	float maxPoints = duration*(float)omxPlayerVideo.GetFPS();
	ofLogVerbose() << "maxPoints: " << maxPoints;
	
	float m_seek_pos = ofMap(pct, 0.0f, 100.0f, 0.0f, maxPoints);
	
	ofLogVerbose() << "m_seek_pos: " << m_seek_pos;
	double startpts;
	ofLog(OF_LOG_VERBOSE, "\nSeeking start of video to %f\n", m_seek_pos);
	omxReader.SeekTime(m_seek_pos, AVSEEK_FLAG_BACKWARD, &startpts);  // from seconds to DVD_TIME_BASE
	ofLogVerbose() << "SEEK RESULT: " << startpts;*/
	double m_incr = 60.0;
	double startpts = 0;
	if(m_incr != 0)
    {
		int    seek_flags   = 0;
		double seek_pos     = 0;
		double pts          = 0;
		

		//clock->OMXStop();
		
		pts = clock->GetPTS();
		
		seek_pos = (pts / DVD_TIME_BASE) + m_incr;
		seek_flags = m_incr < 0.0f ? AVSEEK_FLAG_BACKWARD : 0;
		
		seek_pos *= 1000.0f;
		
		m_incr = 0;
		
		//if()
//		{
//			/*omxPlayerVideo.Flush();
//			if(packet)
//			{
//				omxReader.FreePacket(packet);
//				packet = NULL;
//			}*/
//		}
		
		//omxPlayerVideo.Close();
		//omxPlayerVideo.Open(streamInfo, clock, eglImage);
		omxReader.SeekTime(seek_pos, seek_flags, &startpts);
		//clock->SetPTS(startpts);
		clock->OMXStart(startpts);
		//clock->OMXStart(startpts);
    }
	
	
	
}
void ofxOMXPlayer::setLoopState(ofLoopType state){
	

omxReader.currentLoopState = state;
}

//---------------------------------------------------------------------------
ofLoopType ofxOMXPlayer::getLoopState(){
	return omxReader.currentLoopState;
}

//------------------------------------
int ofxOMXPlayer::getCurrentFrame(){
	return (int)(omxPlayerVideo.GetCurrentPTS()/ DVD_TIME_BASE)*omxPlayerVideo.GetFPS();
}
//------------------------------------
int ofxOMXPlayer::getTotalNumFrames(){
	return nFrames;
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
	//OMXReader::g_abort = true;
	omxReader.Close();
	
	if(packet)
	{
		omxReader.FreePacket(packet);
		packet = NULL;
	}	
	
		
	if (eglImage !=NULL)  
	{
		eglDestroyImageKHR(display, eglImage);
	}

	omxCore.Deinitialize();
	rbp.Deinitialize();
	ofLogVerbose() << "reached end of ofxOMXPlayer::close";
}

double ofxOMXPlayer::getMediaTime()
{
	double mediaTime = 0.0;
	if(isPlaying()) 
	{
		mediaTime =  clock->OMXMediaTime();
	}
	
	return mediaTime;

	
}
