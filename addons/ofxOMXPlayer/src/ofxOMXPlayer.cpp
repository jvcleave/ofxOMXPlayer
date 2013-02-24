#include "ofxOMXPlayer.h"

ofxOMXPlayer::ofxOMXPlayer()
{
	videoWidth			= 0;
	videoHeight			= 0;
	isMPEG				= false;
	hasVideo			= false;
	hasAudio			= false;
	packet				= NULL;
	moviePath			= "moviePath is undefined";
	clock				= NULL;
	playerTex			= NULL;
	bPlaying			= false;
	duration			= 0.0;
	nFrames				= 0;
	doVideoDebugging	= false;
	doLooping			= true;
	isThreaded		= false;
	m_buffer_empty = true;
	if (doVideoDebugging) 
	{
		videoPlayer.doDebugging = true; //this can cause a string error, probably thread safe issue
	}
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
		int audioStreamCount	 = omxReader.AudioStreamCount();
		if (audioStreamCount>0) 
		{
			hasAudio = true;
			ofLogVerbose() << "HAS AUDIO";
		}else 
		{
			ofLogVerbose() << "NO AUDIO";
		}
		if (hasVideo) 
		{
			ofLogVerbose() << "Video streams detection PASS";
			
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
	omxReader.GetHints(OMXSTREAM_AUDIO, audioStreamInfo);

	videoWidth	= streamInfo.width;
	videoHeight	= streamInfo.height;
	ofLogVerbose() << "SET videoWidth: " << videoWidth;
	ofLogVerbose() << "SET videoHeight: " << videoHeight;

	generateEGLImage();
	bPlaying = videoPlayer.Open(streamInfo, clock, eglImage);
	string deviceString			= "omx:local";
	bool m_passthrough			= false;
	int m_use_hw_audio			= false;
	bool m_boost_on_downmix		= false;
	bool m_thread_player		= true;
	bool didAudioOpen = m_player_audio.Open(audioStreamInfo, clock, &omxReader, deviceString, 
											m_passthrough, m_use_hw_audio,
											m_boost_on_downmix, m_thread_player);
	if (didAudioOpen) 
	{
		ofLogVerbose() << " AUDIO PLAYER OPEN PASS";
	}else
	{
		ofLogVerbose() << " AUDIO PLAYER OPEN FAIL";
		
	}
	
	if (isPlaying()) 
	{
		ofLogVerbose() << "streamInfo.nb_frames " << streamInfo.nb_frames;
		ofLogVerbose() << "videoPlayer.GetFPS(): " << videoPlayer.GetFPS();
		
		if(streamInfo.nb_frames>0 && videoPlayer.GetFPS()>0)
		{
			nFrames = streamInfo.nb_frames;
			duration = streamInfo.nb_frames / videoPlayer.GetFPS();
			ofLogVerbose() << "duration SET: " << duration;
			isThreaded = true;
			
		}else 
		{
			
			//file is weird (like test.h264) and has no reported frames
			//enable File looping hack if looping enabled;
			
			if (doLooping) 
			{
				omxReader.enableFileLoopinghack();
			}
		}
		if (isThreaded) 
		{
			ofLogVerbose() << "ofxOMXPlayer THREADING ENABLED";
			startThread(false, true);
		}else 
		{
			ofLogVerbose() << "ofxOMXPlayer THREADING DISABLED";
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
}


ofTexture & ofxOMXPlayer::getTextureReference()
{
	if(playerTex == NULL){
		return tex;
	}
	else{
		return *playerTex;
	}
}
void ofxOMXPlayer::threadedFunction()
{
	while (isThreadRunning()) 
	{
		update();
	}
}
void ofxOMXPlayer::update()
{
	if(!isPlaying())
	{
		return;
	}
	if (doLooping && !packet) 
	{
		if (!omxReader.IsEof())
		{
			packet = omxReader.Read(false);
		}else 
		{
			/*if (!hasPrintedEOF) {
				hasPrintedEOF = true;
				ofLogVerbose() << "READER IS EOF";
			}*/
			bool isAudioCacheEmpty = true;
			if (hasAudio) 
			{
				if (doVideoDebugging) 
				{
					ofLogVerbose() << "Audio Cache size: " << m_player_audio.GetCached();
				}
					
				isAudioCacheEmpty = m_player_audio.GetCached()>0;
			}
			if (doVideoDebugging) 
			{
				ofLogVerbose() << "Video Cache size: " << videoPlayer.GetCached();

			}
			if (!isAudioCacheEmpty && !videoPlayer.GetCached())
			{
				double startpts;
				
				
				ofLogVerbose() << "looping via doLooping option";
				if (hasAudio) 
				{
					m_player_audio.WaitCompletion();
				}
				videoPlayer.WaitCompletion();
				if(omxReader.SeekTime(0 * 1000.0f, 0, &startpts))
				{
					videoPlayer.Flush();
					if(hasAudio)
					{
						m_player_audio.Flush();
					}
					if(packet)
					{
						omxReader.FreePacket(packet);
						packet = NULL;
					}
					
					if(startpts != DVD_NOPTS_VALUE)
					{
						clock->OMXUpdateClock(startpts);
						
					}
					videoPlayer.UnFlush();	
				}
				
				
				//packet = omxReader.Read(true);
			}
		}
	}else 
	{
		if(!packet)
		{
			packet = omxReader.Read(false);
		}
	}

	if(packet && omxReader.IsActive(OMXSTREAM_VIDEO, packet->stream_index))
    {
		if(videoPlayer.AddPacket(packet))
		{
			packet = NULL;
		}else 
		{
			OMXClock::OMXSleep(10);
		}
	}else 
	{
		if(hasAudio && packet && packet->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			if(m_player_audio.AddPacket(packet))
			{
				packet = NULL;
			}
			else
			{
				OMXClock::OMXSleep(10);
			}
		}
		if(packet)
		{
			omxReader.FreePacket(packet);
			packet = NULL;
		}
	}
	

}
//--------------------------------------------------------
void ofxOMXPlayer::play()
{
	ofLogVerbose() << "TODO: not sure what to do with this - reopen the player?";
	/*clock->SetSpeed(OMX_PLAYSPEED_NORMAL);
	clock->OMXStateExecute();
	clock->OMXStart();
	bPlaying = openPlayer();*/
}

void ofxOMXPlayer::stop()
{
	clock->OMXStop();
	clock->OMXStateIdle();
	videoPlayer.Close();
	bPlaying = false;
	ofLogVerbose() << "ofxOMXPlayer::stop called";
}

void ofxOMXPlayer::setPaused(bool doPause)
{
	if(doPause)
	{
		videoPlayer.SetSpeed(OMX_PLAYSPEED_PAUSE);
		clock->OMXPause();
	}else 
	{
		videoPlayer.SetSpeed(OMX_PLAYSPEED_NORMAL);
		clock->OMXResume();
	}

			
}
void ofxOMXPlayer::draw(float x, float y, float width, float height)
{
	if(!isPlaying()) return;
	getTextureReference().draw(x, y, width, height);	
}

void ofxOMXPlayer::draw(float x, float y)
{
	if(!isPlaying()) return;
	getTextureReference().draw(x, y);
}

string ofxOMXPlayer::getVideoDebugInfo()
{
	if (!doVideoDebugging) 
	{
		return "doVideoDebugging not enabled";
	}
	if (videoPlayer.m_decoder != NULL) 
	{
		return videoPlayer.m_decoder->debugInfo + "\n" + videoPlayer.debugInfo;
	}
	return "NO INFO YET";
}

float ofxOMXPlayer::getDuration()
{
	return duration;
}
//inaccurate, I believe as it is referring to decoded frames which can be 
//ahead of rendered frames
int ofxOMXPlayer::getCurrentFrame()
{
	return (int)(videoPlayer.GetCurrentPTS()/ DVD_TIME_BASE)*videoPlayer.GetFPS();
}

int ofxOMXPlayer::getTotalNumFrames()
{
	return nFrames;
}

float ofxOMXPlayer::getWidth()
{
	return videoWidth;
}

float ofxOMXPlayer::getHeight()
{
	return videoHeight;
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

bool ofxOMXPlayer::isPaused()
{
	if (!clock) 
	{
		ofLogVerbose() << "No clock for pause state inquiry";
		return false;
	}
	return clock->OMXIsPaused();
}

bool ofxOMXPlayer::isPlaying()
{
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
