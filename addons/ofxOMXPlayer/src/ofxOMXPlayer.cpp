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
	bPlaying			= false;
	duration			= 0.0;
	nFrames				= 0;
	doVideoDebugging	= false;
	doLooping			= true;
	isBufferEmpty		= true;
	loop_offset = 0;
	startpts              = 0.0;
	if (doVideoDebugging) 
	{
		//videoPlayer->doDebugging = true; //this can cause a string error, probably thread safe issue
	}
	//m_stop				= false;
	didAudioOpen		= false;
	didVideoOpen		= false;
	isTextureEnabled	= false;
	videoPlayer			= NULL;
	
	rbp.Initialize();
	omxCore.Initialize();
	
	clock = new OMXClock(); 
	eglPlayer = NULL;
	nonEglPlayer = NULL;
	ofAddListener(ofEvents().exit, this, &ofxOMXPlayer::close);
}


void ofxOMXPlayer::setup(ofxOMXPlayerSettings settings_)
{
	settings = settings_;
	moviePath = settings.videoPath; 
	ofLogVerbose() << "moviePath is " << moviePath;
	isTextureEnabled = settings.enableTexture;
	
	
	bool doDumpFormat = false;
	
	if(omxReader.Open(moviePath.c_str(), doDumpFormat))
	{
		
		ofLogVerbose() << "omxReader open moviePath PASS: " << moviePath;
		
		hasVideo = omxReader.VideoStreamCount();
		int audioStreamCount = omxReader.AudioStreamCount();
		
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
			ofLogVerbose()	<< "Video streams detection PASS";
			
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
	omxReader.GetHints(OMXSTREAM_VIDEO, videoStreamInfo);
	omxReader.GetHints(OMXSTREAM_AUDIO, audioStreamInfo);

	videoWidth	= videoStreamInfo.width;
	videoHeight = videoStreamInfo.height;
	
	ofLogVerbose() << "SET videoWidth: "	<< videoWidth;
	ofLogVerbose() << "SET videoHeight: "	<< videoHeight;
	ofLogVerbose() << "videoStreamInfo.nb_frames " <<videoStreamInfo.nb_frames;
	if (isTextureEnabled) 
	{
		if (!eglPlayer) 
		{
			generateEGLImage();
			eglPlayer = new OMXPlayerEGLImage();
		}
		
		didVideoOpen = eglPlayer->Open(videoStreamInfo, clock, eglImage);
		videoPlayer = (OMXPlayerVideoBase*)eglPlayer;

	}else 
	{
		if (!nonEglPlayer) {
			nonEglPlayer = new OMXPlayerVideo();
		}
		bool deinterlace = false;
		bool hdmi_clock_sync = true;
		float display_aspect = 1.0;
		
		didVideoOpen = nonEglPlayer->Open(videoStreamInfo, clock, deinterlace, hdmi_clock_sync, display_aspect);
		videoPlayer = (OMXPlayerVideoBase*)nonEglPlayer;
		
	}
	
	bPlaying = didVideoOpen;
	
	if (hasAudio) 
	{
		string deviceString = "omx:hdmi";
		if (!settings.useHDMIForAudio)
		{
			deviceString = "omx:local";
		}
		bool m_passthrough			= false;
		int m_use_hw_audio			= false;
		bool m_boost_on_downmix		= false;
		bool m_thread_player		= true;
		didAudioOpen = audioPlayer.Open(audioStreamInfo, clock, &omxReader, deviceString, 
										m_passthrough, m_use_hw_audio,
										m_boost_on_downmix, m_thread_player);
		if (didAudioOpen) 
		{
			ofLogVerbose() << " AUDIO PLAYER OPEN PASS";
		}else
		{
			ofLogVerbose() << " AUDIO PLAYER OPEN FAIL";
			
		}
	}
	
	
	if (isPlaying()) 
	{
		
		ofLogVerbose() << "videoPlayer->GetFPS(): " << videoPlayer->GetFPS();
		
		if(videoStreamInfo.nb_frames>0 && videoPlayer->GetFPS()>0)
		{
			nFrames =videoStreamInfo.nb_frames;
			duration =videoStreamInfo.nb_frames / videoPlayer->GetFPS();
			ofLogVerbose() << "duration SET: " << duration;			
		}else 
		{
			
			//file is weird (like test.h264) and has no reported frames
			//enable File looping hack if looping enabled;
			
			if (doLooping) 
			{
				//omxReader.enableFileLoopinghack();
			}
		}
		clock->OMXStateExecute();
		clock->OMXStart(0.0);
		startThread(false, true);
		
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
	return tex;
}
void ofxOMXPlayer::threadedFunction()
{
	while (isThreadRunning()) 
	{
		struct timespec starttime, endtime;
		/*printf("V : %8.02f %8d %8d A : %8.02f %8.02f Cv : %8d Ca : %8d                            \r",
		 clock->OMXMediaTime(), videoPlayer->GetDecoderBufferSize(),
		 videoPlayer->GetDecoderFreeSpace(), audioPlayer.GetCurrentPTS() / DVD_TIME_BASE, 
		 audioPlayer.GetDelay(), videoPlayer->GetCached(), audioPlayer.GetCached());*/
		
		if(omxReader.IsEof() && !packet)
		{
			if (!audioPlayer.GetCached() && !videoPlayer->GetCached())
			{
				if (doLooping)
				{
					//startpts = 1.0;
					omxReader.SeekTime(0 * 1000.0f, AVSEEK_FLAG_BACKWARD, &startpts);
					if(hasAudio)
					{
						loop_offset = audioPlayer.GetCurrentPTS() /* + DVD_MSEC_TO_TIME(0) */;
					}
					else if(hasVideo)
					{
						loop_offset = videoPlayer->GetCurrentPTS();
						
					} 
					ofLog(OF_LOG_VERBOSE, "Loop offset : %8.02f\n", loop_offset / DVD_TIME_BASE);
				}
				else
				{
					break;
				}
			}
			else
			{
				OMXClock::OMXSleep(10);
				continue;
			}
		}
		
		if (hasAudio) 
		{
			if(audioPlayer.Error())
			 {
				 ofLogError(__func__) << "audio player error.";
			 }
		}
		
		if(!packet)
		{
			packet = omxReader.Read();
			if (packet && doLooping && packet->pts != DVD_NOPTS_VALUE)
			{
				packet->pts += loop_offset;
				packet->dts += loop_offset;
			}
		}
		
		if(hasVideo && packet && omxReader.IsActive(OMXSTREAM_VIDEO, packet->stream_index))
		{
			if(videoPlayer->AddPacket(packet))
			{
				packet = NULL;
			}
			else
			{
				OMXClock::OMXSleep(10);
			}
		}
		else if(hasAudio && packet && packet->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			if(audioPlayer.AddPacket(packet))
			{
				packet = NULL;
			}
			else
			{
				OMXClock::OMXSleep(10);
			}
		}
		else
		{
			if(packet)
			{
				omxReader.FreePacket(packet);
				packet = NULL;
			}
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
	/*clock->OMXStop();
	clock->OMXStateIdle();
	videoPlayer->Close();
	bPlaying = false;
	ofLogVerbose() << "ofxOMXPlayer::stop called";*/
}

void ofxOMXPlayer::setPaused(bool doPause)
{
	ofLogVerbose(__func__) << " doPause: " << doPause;
	if(doPause)
	{
		clock->OMXPause();
	}else 
	{
		clock->OMXResume();
	}
			
}
void ofxOMXPlayer::draw(float x, float y, float width, float height)
{
	if(!isPlaying()) return;
	if (!isTextureEnabled) 
	{
		return;
	}
	getTextureReference().draw(x, y, width, height);	
}

void ofxOMXPlayer::draw(float x, float y)
{
	if(!isPlaying()) return;
	if (!isTextureEnabled) 
	{
		return;
	}
	getTextureReference().draw(x, y);
}

string ofxOMXPlayer::getVideoDebugInfo()
{
	/*if (!doVideoDebugging) 
	{
		return "doVideoDebugging not enabled";
	}
	if (videoPlayer->m_decoder != NULL) 
	{
		return videoPlayer->m_decoder->debugInfo + "\n" + videoPlayer->debugInfo;
	}*/
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
	return (int)(videoPlayer->GetCurrentPTS()/ DVD_TIME_BASE)*videoPlayer->GetFPS();
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
	return clock->OMXIsPaused();
}

bool ofxOMXPlayer::isPlaying()
{
	return bPlaying;
}

void ofxOMXPlayer::close(ofEventArgs & a)
{
	waitForThread(true);
	//setPaused(true);
	sleep(1000);
	OMXClock::OMXSleep(1000);
	sleep(1000);
	ofLogVerbose() << "start ofxOMXPlayer::close";
	if (isTextureEnabled) 
	{
		OMXClock::OMXSleep(500);
	}

	if (!isTextureEnabled) 
	{
		clock->OMXStop();
		ofLogVerbose(__func__) << "clock->OMXStopped";
		clock->OMXStateIdle();
		ofLogVerbose(__func__) << "clock->OMXStateIdle";
	}


	videoPlayer->Close();
	ofLogVerbose(__func__) << "videoPlayer->Closed";
	audioPlayer.Close();
	ofLogVerbose(__func__) << "audioPlayer->Closed";
	if(packet)
	{
		omxReader.FreePacket(packet);
		packet = NULL;
	}
	ofLogVerbose(__func__) << "packet freed";
	omxReader.Close();
	ofLogVerbose(__func__) << "omxReader->Closed";
	
	if (eglImage !=NULL)  
	{
		eglDestroyImageKHR(display, eglImage);
		ofLogVerbose(__func__) << "eglImage->Destroyed";
	}
	
	omxCore.Deinitialize();
	ofLogVerbose(__func__) << "omxCore->Deinitialized";
	rbp.Deinitialize();
	ofLogVerbose(__func__) << "rbp->Deinitialized";
	delete videoPlayer;
	ofLogVerbose(__func__) << "videoPlayer->deleted";
	videoPlayer = NULL;
	ofLogVerbose(__func__) << "videoPlayer->NULL";
	ofLogVerbose() << "reached end of ofxOMXPlayer::close";
	ofRemoveListener(ofEvents().exit, this, &ofxOMXPlayer::close);
}
