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
	loopCounter = 0;
	doAbort = false;
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

bool doLoopReset = false;
void ofxOMXPlayer::threadedFunction()
{
	while (isThreadRunning()) 
	{
		if (doAbort) 
		{
			return;
		}
		//struct timespec starttime, endtime;
		/*printf("V : %8.02f %8d %8d A : %8.02f %8.02f Cv : %8d Ca : %8d                            \r",
		 clock->OMXMediaTime(), videoPlayer->GetDecoderBufferSize(),
		 videoPlayer->GetDecoderFreeSpace(), audioPlayer.GetCurrentPTS() / DVD_TIME_BASE, 
		 audioPlayer.GetDelay(), videoPlayer->GetCached(), audioPlayer.GetCached());*/
		
		if(omxReader.IsEof() && !packet)
		{
			//ofLogVerbose() << "Dumping Cache " << "Audio Cache: " << audioPlayer.GetCached() << " Video Cache: " << videoPlayer->GetCached();
			if (!audioPlayer.GetCached() && !videoPlayer->GetCached())
			{
				/*
				 The way this works is that loop_offset is a marker (actually the same as the DURATION)
				 Once the file reader seeks to the beginning of the file again loop_offset is then added to subsequent packet's timestamps
				 */
				if (doLooping)
				{
				
					if(hasVideo)
					{
						videoPlayer->WaitCompletion();
					}
					omxReader.SeekTime(0 * 1000.0f, AVSEEK_FLAG_BACKWARD, &startpts);
					if(hasAudio)
					{
						loop_offset = audioPlayer.GetCurrentPTS();
						ofLogVerbose() << "LOOP via audioPlayer [] [] [] [] [] [] [] []";

					}
					else if(hasVideo)
					{
						loop_offset = videoPlayer->GetCurrentPTS();
						ofLogVerbose() << "LOOP via videoPlayer [] [] [] [] [] [] [] []";
						
					}
					loopCounter++;

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
		
		if (doLooping && OMXDecoderBase::fillBufferCounter>=getTotalNumFrames()) 
		{
			OMXDecoderBase::fillBufferCounter=0;
			//setPaused(true);
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
//may be inaccurate as we are counting our own frames
int ofxOMXPlayer::getCurrentFrame()
{
	
	return OMXDecoderBase::fillBufferCounter;
	//return (int)(videoPlayer->m_iCurrentPts/ DVD_TIME_BASE)*videoPlayer->GetFPS();
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
void ofxOMXPlayer::stepFrameForward()
{
	if (!isPaused()) 
	{
		setPaused(true);
	}
	clock->OMXStep(1);
}


bool ofxOMXPlayer::isPlaying()
{
	return bPlaying;
}


void ofxOMXPlayer::setVolume(float volume)
{
	ofLogVerbose(__func__) << "volume: " << volume;
	float value = ofMap(volume, 0.0, 1.0, -6000, 6000, false);
	if (hasAudio && didAudioOpen) 
	{
		audioPlayer.SetCurrentVolume(value);
	}
}

float ofxOMXPlayer::getVolume()
{
	if (hasAudio && didAudioOpen) 
	{
		float value = ofMap(audioPlayer.GetCurrentVolume(), -6000, 6000, 0.0, 1.0, false);
		return value;
	}else 
	{
		return 0;
		ofLogError(__func__) << "returning 0 hasAudio: " << hasAudio << " didAudioOpen: " << didAudioOpen;
	}

}
void ofxOMXPlayer::close(ofEventArgs & a)
{
	doAbort = true;
	waitForThread(true);
	//setPaused(true);
	OMXClock::OMXSleep(200);
	ofLogVerbose() << "start ofxOMXPlayer::close";
	if (isTextureEnabled) 
	{
		OMXClock::OMXSleep(200);
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
	delete videoPlayer;
	ofLogVerbose(__func__) << "videoPlayer->deleted";
	videoPlayer = NULL;
	ofLogVerbose(__func__) << "videoPlayer->NULL";
	
	audioPlayer.Close();
	ofLogVerbose(__func__) << "audioPlayer->Closed";
	
	if(packet)
	{
		omxReader.FreePacket(packet);
		packet = NULL;
	}
	ofLogVerbose(__func__) << "packet freed";
	

	
	
	audioPlayer.Close();
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

	
	ofRemoveListener(ofEvents().exit, this, &ofxOMXPlayer::close);
	ofLogVerbose() << "reached end of ofxOMXPlayer::close";
}
