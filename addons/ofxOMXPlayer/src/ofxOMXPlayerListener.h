#pragma once

#include "ofMain.h"

class ofxOMXPlayerListenerEventData
{
public:
	ofxOMXPlayerListenerEventData(void* listener_)
	{
		listener = listener_;
	}
	void* listener;
};


class ofxOMXPlayerListener
{
public:
	virtual void onVideoEnd(ofxOMXPlayerListenerEventData& e) = 0;
	virtual void onVideoLoop(ofxOMXPlayerListenerEventData& e) = 0;
	
};


struct ofxOMXPlayerSettings 
{
	ofxOMXPlayerSettings()
	{
		videoPath = "";
		
		useHDMIForAudio = true;
		enableTexture = true;
		enableLooping = true;
		listener	  = NULL;
	}
	string videoPath;
	bool enableTexture;
	bool useHDMIForAudio;
	bool enableLooping;
	ofxOMXPlayerListener* listener;
	/*
	 To use HDMI Audio you may need to add the below line to /boot/config.txt and reboot
	 
	 hdmi_drive=2
	 
	 see http://elinux.org/RPiconfig for more details
	 */
	
};
