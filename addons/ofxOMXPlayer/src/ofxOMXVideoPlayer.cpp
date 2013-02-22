#include "ofxOMXVideoPlayer.h"

ofxOMXVideoPlayer::ofxOMXVideoPlayer()
{
	videoWidth			= 0;
	videoHeight			= 0;
	isMPEG				= false;
	hasVideo			= false;
	packet				= NULL;
	moviePath			= "moviePath is undefined";
	clock				= NULL;
	bPlaying			= false;
	duration			= 0.0;
	nFrames				= 0;
	doLooping  = true;
}

void ofxOMXVideoPlayer::loadMovie(string filepath)
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
void ofxOMXVideoPlayer::openPlayer()
{
	omxReader.GetHints(OMXSTREAM_VIDEO, streamInfo);
	videoWidth	= streamInfo.width;
	videoHeight	= streamInfo.height;
	ofLogVerbose() << "SET videoWidth: " << videoWidth;
	ofLogVerbose() << "SET videoHeight: " << videoHeight;

	bPlaying = videoPlayer.Open(streamInfo, clock, false, false, false, true, 0.0);
	if (isPlaying()) 
	{
		ofLogVerbose() << "streamInfo.nb_frames " << streamInfo.nb_frames;
		ofLogVerbose() << "videoPlayer.GetFPS(): " << videoPlayer.GetFPS();
		
		if(streamInfo.nb_frames>0 && videoPlayer.GetFPS()>0)
		{
			nFrames = streamInfo.nb_frames;
			duration = streamInfo.nb_frames / videoPlayer.GetFPS();
			ofLogVerbose() << "duration SET: " << duration;
		}else 
		{
			//file is weird (like test.h264) and has no reported frames
			//enable File looping hack if looping enabled;
			
			if (doLooping) 
			{
				omxReader.enableFileLoopinghack();
			}
		}
		clock->SetSpeed(DVD_PLAYSPEED_NORMAL);
		clock->OMXStateExecute();
		clock->OMXStart();
		
		ofLogVerbose() << "Opened video PASS";	
		startThread(false, true);
		
	}else 
	{
		ofLogError() << "Opened video FAIL";
	}
}

void ofxOMXVideoPlayer::threadedFunction()
{
	while (isThreadRunning()) 
	{
		update();
	}
}
void ofxOMXVideoPlayer::update()
{
	
	if(!isPlaying())
	{
		return;
	}
	if (doLooping) 
	{
		if (!omxReader.IsEof())
		{
			packet = omxReader.Read(false);
		}else 
		{
			if (!videoPlayer.GetCached())
			{
				ofLogVerbose() << "looping via doLooping option";
				videoPlayer.Flush();
				videoPlayer.UnFlush();
				packet = omxReader.Read(true);
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
		if(packet)
		{
			omxReader.FreePacket(packet);
			packet = NULL;
		}
	}

}
//--------------------------------------------------------
void ofxOMXVideoPlayer::play()
{
	ofLogVerbose() << "TODO: not sure what to do with this - reopen the player?";
	/*clock->SetSpeed(OMX_PLAYSPEED_NORMAL);
	clock->OMXStateExecute();
	clock->OMXStart();
	bPlaying = openPlayer();*/
}

void ofxOMXVideoPlayer::stop()
{
	clock->OMXStop();
	clock->OMXStateIdle();
	videoPlayer.Close();
	bPlaying = false;
	ofLogVerbose() << "ofxOMXVideoPlayer::stop called";
}

void ofxOMXVideoPlayer::setPaused(bool doPause)
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

float ofxOMXVideoPlayer::getDuration()
{
	return duration;
}
//inaccurate, I believe as it is referring to decoded frames which can be 
//ahead of rendered frames
int ofxOMXVideoPlayer::getCurrentFrame()
{
	return (int)(videoPlayer.GetCurrentPTS()/ DVD_TIME_BASE)*videoPlayer.GetFPS();
}

int ofxOMXVideoPlayer::getTotalNumFrames()
{
	return nFrames;
}

float ofxOMXVideoPlayer::getWidth()
{
	return videoWidth;
}

float ofxOMXVideoPlayer::getHeight()
{
	return videoHeight;
}

double ofxOMXVideoPlayer::getMediaTime()
{
	double mediaTime = 0.0;
	if(isPlaying()) 
	{
		mediaTime =  clock->OMXMediaTime();
	}
	
	return mediaTime;
}

bool ofxOMXVideoPlayer::isPaused()
{
	if (!clock) 
	{
		ofLogVerbose() << "No clock for pause state inquiry";
		return false;
	}
	return clock->OMXIsPaused();
}

bool ofxOMXVideoPlayer::isPlaying()
{
	return bPlaying;
}

void ofxOMXVideoPlayer::close()
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
	

	omxCore.Deinitialize();
	rbp.Deinitialize();
	ofLogVerbose() << "reached end of ofxOMXVideoPlayer::close";
}
