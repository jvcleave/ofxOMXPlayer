#include "ofxOMXPlayerEngine.h"

ofxOMXPlayerEngine::ofxOMXPlayerEngine()
{
	
	
	videoWidth			= 0;
	videoHeight			= 0;
	hasVideo			= false;
	hasAudio			= false;
	packet				= NULL;
	moviePath			= "moviePath is undefined";
	
	bPlaying			= false;
	duration			= 0.0;
	nFrames				= 0;

	useHDMIForAudio		= true;
	loop_offset = 0;
	startpts              = 0.0;

	didAudioOpen		= false;
	didVideoOpen		= false;
	isTextureEnabled	= false;
	doLooping			= false;
	videoPlayer			= NULL;
	eglPlayer			= NULL;
	nonEglPlayer		= NULL;
	
	audioPlayer			= NULL;
	
	listener			= NULL;
	
	//clock				= NULL;
	loopCounter			= 0;
	previousLoopOffset = 0;
	omxCore.Initialize();
	OMXDecoderBase::fillBufferCounter=0;
	
	normalPlaySpeed = 1000;
	speedMultiplier = 1;
	doSeek = false;
	isExiting = false;
	
	eglImage = NULL;
	
}

ofxOMXPlayerEngine::~ofxOMXPlayerEngine()
{
	ofLogVerbose(__func__) << " START";
	ofLogVerbose(__func__) << " isExiting: " << isExiting;
	
	//Lock();
	m_bStop = true;
	
	if(ThreadHandle())
	{	
		StopThread("ofxOMXPlayerEngine");
	}
	
	bPlaying = false;
	
	
	
	if (listener) 
	{
		listener = NULL;
	}
	
	if (eglPlayer) 
	{
		delete eglPlayer;
		eglPlayer = NULL;
	}
	if (nonEglPlayer) 
	{
		delete nonEglPlayer;
		nonEglPlayer = NULL;
	}

	videoPlayer = NULL;
	

	if (audioPlayer) 
	{
		delete audioPlayer;
		audioPlayer = NULL;
	}
	
	if(packet)
	{
		omxReader.FreePacket(packet);
		packet = NULL;
	}
	
	omxReader.Close();
	
	clock.OMXDeinitialize();
	
	
	if (isExiting) 
	{
		omxCore.Deinitialize();
	}
	
	ofLogVerbose(__func__) << "~ofxOMXPlayerEngine END";
	
	
}

void ofxOMXPlayerEngine::startExit()
{
	ofLogVerbose(__func__) << "START";
	isExiting = true;
	
	if (videoPlayer) {
		videoPlayer->isExiting = true;
	}
	ofLogVerbose(__func__) << "END";
}

void ofxOMXPlayerEngine::setNormalSpeed()
{
	
	speedMultiplier = 1;
	clock.OMXSetSpeed(normalPlaySpeed);
	omxReader.SetSpeed(normalPlaySpeed);
	ofLogVerbose(__func__) << "clock speed: " << clock.OMXPlaySpeed();
	ofLogVerbose(__func__) << "reader speed: " << omxReader.GetSpeed();
}

void ofxOMXPlayerEngine::fastForward()
{
	
	Lock();
	ofLogVerbose(__func__) << " START";
	doSeek = true;
	UnLock();
	return;
	ofLogVerbose(__func__) << "clock speed: " << clock.OMXPlaySpeed();
	ofLogVerbose(__func__) << "reader speed: " << omxReader.GetSpeed();
	speedMultiplier++;
	if(speedMultiplier>8)
	{
		speedMultiplier = 1;
	}
	int newSpeed = normalPlaySpeed*speedMultiplier;
	
	clock.OMXSetSpeed(newSpeed);
	omxReader.SetSpeed(newSpeed);
	ofLogVerbose(__func__) << "newSpeed: " << newSpeed;
	
}

void ofxOMXPlayerEngine::rewind()
{
	ofLogVerbose(__func__) << "clock speed: " << clock.OMXPlaySpeed();
	ofLogVerbose(__func__) << "reader speed: " << omxReader.GetSpeed();
	if(speedMultiplier-1 == 0)
	{
		speedMultiplier = -1;
	}else 
	{
		speedMultiplier--;
	}

	
	if(speedMultiplier<-8)
	{
		speedMultiplier = 1;
	}
	int newSpeed = normalPlaySpeed*speedMultiplier;
	
	clock.OMXSetSpeed(newSpeed);
	omxReader.SetSpeed(newSpeed);
	ofLogVerbose(__func__) << "newSpeed: " << newSpeed;
	
}


bool ofxOMXPlayerEngine::setup(ofxOMXPlayerSettings& settings)
{
	omxPlayerSettings		= settings;
	moviePath				= omxPlayerSettings.videoPath; 
	useHDMIForAudio			= omxPlayerSettings.useHDMIForAudio;
	doLooping				= omxPlayerSettings.enableLooping;
	addListener(omxPlayerSettings.listener);
	
	ofLogVerbose(__func__) << "moviePath is " << moviePath;
	isTextureEnabled		= omxPlayerSettings.enableTexture;
	
	
	
	bool doDumpFormat = false;
	
	if(omxReader.Open(moviePath.c_str(), doDumpFormat))
	{
		
		ofLogVerbose(__func__) << "omxReader open moviePath PASS: " << moviePath;
		
		hasVideo = omxReader.VideoStreamCount();
		int audioStreamCount = omxReader.AudioStreamCount();
		
		if (audioStreamCount>0) 
		{
			hasAudio = true;
			ofLogVerbose(__func__) << "HAS AUDIO";
		}else 
		{
			ofLogVerbose(__func__) << "NO AUDIO";
		}
		if (!omxPlayerSettings.enableAudio) 
		{
			hasAudio = false;
		}
		if (hasVideo) 
		{
			ofLogVerbose(__func__)	<< "Video streams detection PASS";
			
			if(clock.OMXInitialize(hasVideo, hasAudio))
			{
				ofLogVerbose(__func__) << "clock Init PASS";
				omxReader.GetHints(OMXSTREAM_VIDEO, videoStreamInfo);
				omxReader.GetHints(OMXSTREAM_AUDIO, audioStreamInfo);
				
				videoWidth	= videoStreamInfo.width;
				videoHeight = videoStreamInfo.height;
				omxPlayerSettings.videoWidth	= videoStreamInfo.width;
				omxPlayerSettings.videoHeight	= videoStreamInfo.height;
				
				ofLogVerbose(__func__) << "SET videoWidth: "	<< videoWidth;
				ofLogVerbose(__func__) << "SET videoHeight: "	<< videoHeight;
				ofLogVerbose(__func__) << "videoStreamInfo.nb_frames " <<videoStreamInfo.nb_frames;
				
				return true;
				
			}else 
			{
				ofLogError() << "clock Init FAIL";
				return false;
			}
		}else 
		{
			ofLogError() << "Video streams detection FAIL";
			return false;
		}
	}else 
	{
		ofLogError() << "omxReader open moviePath FAIL: "  << moviePath;
		return false;
	}
}

bool ofxOMXPlayerEngine::openPlayer()
{
	
	if (isTextureEnabled) 
	{
		if (!eglPlayer) 
		{
			eglPlayer = new OMXPlayerEGLImage();
		}
		//GlobalEGLContainer::getInstance().setup(omxPlayerSettings);
		didVideoOpen = eglPlayer->Open(videoStreamInfo, &clock, eglImage);
		videoPlayer = (OMXPlayerVideoBase*)eglPlayer;
	}else 
	{
		if (!nonEglPlayer) 
		{
			nonEglPlayer = new OMXPlayerVideo();
		}
		bool deinterlace = false;
		bool hdmi_clock_sync = true;
		float display_aspect = 1.0;
		
		didVideoOpen = nonEglPlayer->Open(videoStreamInfo, &clock, deinterlace, hdmi_clock_sync, display_aspect);
		videoPlayer = (OMXPlayerVideoBase*)nonEglPlayer;
		
	}
	
	bPlaying = didVideoOpen;
	
	if (hasAudio) 
	{
		string deviceString = "omx:hdmi";
		if (!useHDMIForAudio)
		{
			deviceString = "omx:local";
		}
		bool m_passthrough			= false;/* passthrough overwrites hw decode */
		int m_use_hw_audio			= false;
		bool m_boost_on_downmix		= false;
		bool m_thread_player		= true;
		audioPlayer = new OMXPlayerAudio();
		didAudioOpen = audioPlayer->Open(audioStreamInfo, &clock, &omxReader, deviceString, 
										m_passthrough, m_use_hw_audio,
										m_boost_on_downmix, m_thread_player);
		if (didAudioOpen) 
		{
			ofLogVerbose(__func__) << " AUDIO PLAYER OPEN PASS";
		}else
		{
			ofLogVerbose(__func__) << " AUDIO PLAYER OPEN FAIL";
		}
	}
	
	
	if (isPlaying()) 
	{
		
		ofLogVerbose(__func__) << "videoPlayer->GetFPS(): " << videoPlayer->GetFPS();
		
		if(videoStreamInfo.nb_frames>0 && videoPlayer->GetFPS()>0)
		{
			nFrames =videoStreamInfo.nb_frames;
			duration =videoStreamInfo.nb_frames / videoPlayer->GetFPS();
			ofLogVerbose(__func__) << "duration SET: " << duration;			
		}else 
		{
			
			//file is weird (like test.h264) and has no reported frames
			//enable File looping hack if looping enabled;
			
			//if (GlobalEGLContainer::getInstance().doLooping) 
//			{
//				//omxReader.enableFileLoopinghack();
//			}
		}
		clock.OMXStateExecute();
		clock.OMXStart(0.0);
				
		ofLogVerbose(__func__) << "Opened video PASS";
		Create();
		return true;
	}else 
	{
		ofLogError() << "Opened video FAIL";
		return false;
	}
}

void ofxOMXPlayerEngine::Process()
{
	
	int freezeFrameCounter = 0;
	int previousFrameNumber = 0;
	while (!m_bStop) 
	{
		
		if(!packet)
		{
			packet = omxReader.Read();
			if (packet && doLooping && packet->pts != DVD_NOPTS_VALUE)
			{
				packet->pts += loop_offset;
				packet->dts += loop_offset;
			}
		}
		
		bool isCacheEmpty = false;
		
		if (!packet) 
		{
			if (hasAudio) 
			{
				if (!audioPlayer->GetCached() && !videoPlayer->GetCached()) 
				{
					isCacheEmpty = true;
				}
			}else 
			{
				if (!videoPlayer->GetCached()) 
				{
					isCacheEmpty = true;
				}
			}
			
		}
		ofLogVerbose(__func__) << "remainingFrames: " << getTotalNumFrames() - getCurrentFrame();
		if(doLooping && omxReader.IsEof() && !packet) 
		{
		
			
			if (isCacheEmpty)
			{
				omxReader.SeekTime(0 * 1000.0f, AVSEEK_FLAG_BACKWARD, &startpts);
				packet = omxReader.Read();
				
				if(hasAudio)
				{
					loop_offset = audioPlayer->GetCurrentPTS();						
				}
				else 
				{
					if(hasVideo)
					{
						loop_offset = videoPlayer->GetCurrentPTS();
					}
				}
				if (previousLoopOffset != loop_offset) 
				{
					
					previousLoopOffset = loop_offset;
					loopCounter++;
					ofLogVerbose(__func__) << "loopCounter: " << loopCounter;
					ofLog(OF_LOG_VERBOSE, "Loop offset : %8.02f\n", loop_offset / DVD_TIME_BASE);
					onVideoLoop();
					
				}
				if (omxReader.wasFileRewound) 
				{
					omxReader.wasFileRewound = false;
					onVideoLoop();
				}
				
			}else
			{
				OMXClock::OMXSleep(10);
				continue;
			}
			
		}else 
		{
			if (!doLooping && omxReader.IsEof() && !packet && isCacheEmpty)
			{
				//not sure why losing frames here but better than before
				if (!isPaused()) 
				{
					int remainingFrames = getTotalNumFrames() - getCurrentFrame();
					if (remainingFrames <=10) 
					{
						if (previousFrameNumber == 0) 
						{
							previousFrameNumber = remainingFrames;
						}else
						{
							if (previousFrameNumber == remainingFrames) 
							{
								freezeFrameCounter++;
							}
						}
						
						previousFrameNumber = remainingFrames;
						
						ofLogVerbose(__func__) << "remainingFrames: " << remainingFrames;
						ofLogVerbose(__func__) << "freezeFrameCounter: " << freezeFrameCounter;
						ofLogVerbose(__func__) << "previousFrameNumber: " << previousFrameNumber;
						if (freezeFrameCounter>=5) 
						{
							onVideoEnd();
							freezeFrameCounter = 0;
							previousFrameNumber = 0;
							break;
						}
						
					}
				}
			}
		}
		
		
		if (doLooping && OMXDecoderBase::fillBufferCounter>=getTotalNumFrames()) 
		{
			OMXDecoderBase::fillBufferCounter=0;
		}
		if (hasAudio) 
		{
			if(audioPlayer->Error())
			{
				ofLogError(__func__) << "audio player error.";
				hasAudio = false;
			}
		}
		
		/*if(!packet)
		{
			packet = omxReader.Read();
			
		}*/
		
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
			if(audioPlayer->AddPacket(packet))
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
#if 0	
	
		
	
	
	
	
	
	if(m_loop && m_omx_pkt) {
		if(m_omx_pkt->pts != DVD_NOPTS_VALUE) {
			if(m_omx_pkt->pts < loop_offset) {
				CLog::Log(LOGDEBUG, "Applying loop-offset to packet's PTS %.0f->%.0f", m_omx_pkt->pts, m_omx_pkt->pts + loop_offset);
				m_omx_pkt->pts += loop_offset;
			}
			if(m_omx_pkt->pts > last_packet_pts)
				last_packet_pts = m_omx_pkt->pts;
		}
		if(m_omx_pkt->dts != DVD_NOPTS_VALUE) {
			if(m_omx_pkt->dts < loop_offset) {
				CLog::Log(LOGDEBUG, "Applying loop-offset to packet's DTS %.0f->%.0f", m_omx_pkt->dts, m_omx_pkt->dts + loop_offset);
				m_omx_pkt->dts += loop_offset;
			}
			if(m_omx_pkt->dts > last_packet_dts)
				last_packet_dts = m_omx_pkt->dts;
		}
		last_packet_duration = m_omx_pkt->duration;
	}
	
	if(m_omx_reader.IsEof() && !m_omx_pkt)
	{
		// demuxer EOF, but may have not played out data yet
		if ( (m_has_video && m_player_video.GetCached()) ||
			(m_has_audio && m_player_audio.GetCached()) )
		{
			OMXClock::OMXSleep(10);
			continue;
		}
		if (!m_send_eos && m_has_video)
			m_player_video.SubmitEOS();
		if (!m_send_eos && m_has_audio)
			m_player_audio.SubmitEOS();
		m_send_eos = true;
		if ( (m_has_video && !m_player_video.IsEOS()) ||
			(m_has_audio && !m_player_audio.IsEOS()) )
		{
			OMXClock::OMXSleep(10);
			continue;
		}
		break;
	}
	
	while (!m_bStop) 
	{
		//struct timespec starttime, endtime;
		/*printf("V : %8.02f %8d %8d A : %8.02f %8.02f Cv : %8d Ca : %8d                            \r",
		 clock.OMXMediaTime(), videoPlayer->GetDecoderBufferSize(),
		 videoPlayer->GetDecoderFreeSpace(), audioPlayer->GetCurrentPTS() / DVD_TIME_BASE, 
		 audioPlayer->GetDelay(), videoPlayer->GetCached(), audioPlayer->GetCached());*/
		
		if(omxReader.IsEof() && !packet)
		{
			//ofLogVerbose(__func__) << "Dumping Cache " << "Audio Cache: " << audioPlayer->GetCached() << " Video Cache: " << videoPlayer->GetCached();
			ofLogVerbose(__func__)  << "getCurrentFrame: "		<< getCurrentFrame();
			ofLogVerbose(__func__)  <<	"REMAINING FRAMES: "	<< getTotalNumFrames() - getCurrentFrame();
			bool isCacheEmpty = false;
			
			if (hasAudio) 
			{
				if (!audioPlayer->GetCached() && !videoPlayer->GetCached()) 
				{
					isCacheEmpty = true;
				}
			}else 
			{
				if (!videoPlayer->GetCached()) 
				{
					isCacheEmpty = true;
				}
			}
			if (isCacheEmpty)
			{
				
				/*
				 The way this works is that loop_offset is a marker (actually the same as the DURATION)
				 Once the file reader seeks to the beginning of the file again loop_offset is then added to subsequent packet's timestamps
				 */
				
				if (doLooping)
				{										
					omxReader.SeekTime(0 * 1000.0f, AVSEEK_FLAG_BACKWARD, &startpts);
					packet = omxReader.Read();
					if(hasAudio)
					{
						loop_offset = audioPlayer->GetCurrentPTS();						
					}
					else 
					{
						if(hasVideo)
						{
							loop_offset = videoPlayer->GetCurrentPTS();
						}
						
					}
					if (previousLoopOffset != loop_offset) 
					{
						
						previousLoopOffset = loop_offset;
						loopCounter++;
						ofLogVerbose(__func__) << "loopCounter: " << loopCounter;
						ofLog(OF_LOG_VERBOSE, "Loop offset : %8.02f\n", loop_offset / DVD_TIME_BASE);
						onVideoLoop();
						
					}
					if (omxReader.wasFileRewound) 
					{
						omxReader.wasFileRewound = false;
						onVideoLoop();
					}
					
				}
				else
				{
					onVideoEnd();
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
		}
		if (hasAudio) 
		{
			if(audioPlayer->Error())
			{
				ofLogError(__func__) << "audio player error.";
				hasAudio = false;
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
			if(audioPlayer->AddPacket(packet))
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

#endif
//--------------------------------------------------------
void ofxOMXPlayerEngine::play()
{
	ofLogVerbose(__func__) << "TODO: not sure what to do with this - reopen the player?";
}

void ofxOMXPlayerEngine::stop()
{
	ofLogVerbose(__func__) << "TODO: not sure what to do with this - pauses for now";
	setPaused(true);
}

void ofxOMXPlayerEngine::setPaused(bool doPause)
{
	ofLogVerbose(__func__) << " doPause: " << doPause;
	if(doPause)
	{
		
		clock.OMXPause();
	}else 
	{
		
		clock.OMXResume();
	}
			
}



float ofxOMXPlayerEngine::getDuration()
{
	return duration;
}

//we are counting our own frames
int ofxOMXPlayerEngine::getCurrentFrame()
{
	
	return OMXDecoderBase::fillBufferCounter;
}

int ofxOMXPlayerEngine::getTotalNumFrames()
{
	return nFrames;
}

int ofxOMXPlayerEngine::getWidth()
{
	return videoWidth;
}

int ofxOMXPlayerEngine::getHeight()
{
	return videoHeight;
}

double ofxOMXPlayerEngine::getMediaTime()
{
	double mediaTime = 0.0;
	if(isPlaying()) 
	{
		mediaTime =  clock.OMXMediaTime();
	}
	
	return mediaTime;
}

bool ofxOMXPlayerEngine::isPaused()
{
	return clock.OMXIsPaused();
}


void ofxOMXPlayerEngine::stepFrameForward()
{
	if (!isPaused()) 
	{
		setPaused(true);
	}
	clock.OMXStep(1);
}


bool ofxOMXPlayerEngine::isPlaying()
{
	return bPlaying;
}

void ofxOMXPlayerEngine::increaseVolume()
{
	if (!hasAudio || !didAudioOpen) 
	{
		return;
	}
	
	float currentVolume = getVolume();
	currentVolume+=0.1;
	setVolume(currentVolume);
}


void ofxOMXPlayerEngine::decreaseVolume()
{
	if (!hasAudio || !didAudioOpen) 
	{
		return;
	}
	
	float currentVolume = getVolume();
	currentVolume-=0.1;
	setVolume(currentVolume);
}

void ofxOMXPlayerEngine::setVolume(float volume)
{
	if (!hasAudio || !didAudioOpen) 
	{
		return;
	}
	float value = ofMap(volume, 0.0, 1.0, -6000.0, 6000.0, true);
	audioPlayer->SetCurrentVolume(value);
}

float ofxOMXPlayerEngine::getVolume()
{
	if (!hasAudio || !didAudioOpen || !audioPlayer) 
	{
		return 0;
	}
	float value = ofMap(audioPlayer->GetCurrentVolume(), -6000.0, 6000.0, 0.0, 1.0, true);
	return floorf(value * 100 + 0.5) / 100;
}


void ofxOMXPlayerEngine::addListener(ofxOMXPlayerListener* listener_)
{
	listener = listener_;
}

void ofxOMXPlayerEngine::removeListener()
{
	listener = NULL;
}


void ofxOMXPlayerEngine::onVideoLoop()
{
	if (listener != NULL) 
	{
		
		ofxOMXPlayerListenerEventData eventData((void *)this);
		listener->onVideoLoop(eventData);
	}
}
void ofxOMXPlayerEngine::onVideoEnd()
{
	if (listener != NULL) 
	{
		
		ofxOMXPlayerListenerEventData eventData((void *)this);
		listener->onVideoEnd(eventData);
	}
	
}
